#include <stdio.h>
#include <cmath>
#include "example_dll.h"

extern "C" {
    #include "qdbmp.c"
}


__stdcall void run(
                    std::function<void(char*&, size_t&)>       getInput,
                    std::function<void(size_t)>                expandOutput,
                    std::function<void(char*, size_t)>         writeOutput,
                    std::function<void(char*&, size_t&)>       getOutput)
{
    char* s = nullptr;
    size_t size = 0;

    getInput(s, size);

    BMP* img = BMP_ReadBuff((unsigned char*)s, size);
    
    if(!img) {

        std::cout << "Segmentor.Run: Failed to read image: " << BMP_GetErrorDescription() << std::endl;
    }

    uint32_t w = 0, h = 0;

    h = BMP_GetHeight(img);
    w = BMP_GetWidth(img);
    
    BMP* processedImg = BMP_Create(w, h, BMP_GetDepth(img));
    
    // #define filterWidth 5
    // #define filterHeight 5

    // double filter[filterHeight][filterWidth] =
    // {
    // -1,  0,  0,  0,  0,
    // 0, -2,  0,  0,  0,
    // 0,  0,  6,  0,  0,
    // 0,  0,  0, -2,  0,
    // 0,  0,  0,  0, -1,
    // };

    // double factor = 2.0;
    // double bias = 0.0;
        

    #define filterWidth 5
    #define filterHeight 5

    double filter[filterHeight][filterWidth] =
    {
    -1, -1, -1, -1,  0,
    -1, -1, -1,  0,  1,
    -1, -1,  0,  1,  1,
    -1,  0,  1,  1,  1,
    0,  1,  1,  1,  1
    };

    double factor = 1.0;
    double bias = 128.0;

    std::cout << "Segmentor: Running... Image W/H: " << h << ", " << w << std::endl;


    //apply the filter
    for(int x = 0; x < w; x++)
    for(int y = 0; y < h; y++)
    {
        double red = 0.0, green = 0.0, blue = 0.0;

        //multiply every value of the filter with corresponding image pixel
        for(int filterY = 0; filterY < filterHeight; filterY++)
        for(int filterX = 0; filterX < filterWidth; filterX++)
        {
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            int imageX = (x - filterWidth / 2 + filterX + w) % w;
            int imageY = (y - filterHeight / 2 + filterY + h) % h;

            BMP_GetPixelRGB(img, imageX, imageY, &r, &g, &b);

            red += r * filter[filterY][filterX];
            green += g * filter[filterY][filterX];
            blue += b * filter[filterY][filterX];
        }

        BMP_SetPixelRGB(processedImg, x, y, 
                                    std::min(std::max(int(factor * red + bias), 0), 255), 
                                    std::min(std::max(int(factor * green + bias), 0), 255), 
                                    std::min(std::max(int(factor * blue + bias), 0), 255));
    }

    expandOutput(BMP_GetSizeInBytes(processedImg));

    char*   result = nullptr;
    size_t  resultSize   = 0;
    getOutput(result, resultSize);

    BMP_WriteBuff(processedImg, (unsigned char*)result, resultSize);

    FILE* f = fopen("after_run.bmp", "wb");
    fwrite((unsigned char*)result, sizeof(char), resultSize, f);
    fclose(f);

    BMP_Free(img);
    BMP_Free(processedImg);
}

__stdcall void combine(
                        int                                        numSegments,
                        std::function<void(char*&, size_t&, int)>  getSegment,
                        std::function<void(char*&, size_t&)>       getOutput,
                        std::function<void(size_t)>                expandOutput,
                        std::function<void(char*, size_t)>         writeOutput
                    )
{
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t depth = 0;

    std::vector<BMP*> subImgs;

    for(int i = 0; i < numSegments; i++) {
        
        char*   segment = nullptr;
        size_t  size    = 0;
        
        getSegment(segment, size, i);

        std::cout << "Reading Image... " << std::endl;

        BMP* subImg = BMP_ReadBuff((unsigned char*)segment, size);

        if(!subImg) {

            std::cout << "Failed to create subimage!" << std::endl;
            std::cout << BMP_GetErrorDescription() << std::endl;

            return;
        }

        subImgs.push_back(subImg); 

        std::cout << "Finished Reading Image... " << std::endl;
    }

    for(int i = 0; i < numSegments; i++) {

        depth = BMP_GetDepth(subImgs[i]);

        height += BMP_GetHeight(subImgs[i]);
        width += BMP_GetWidth(subImgs[i]);

        std::cout << "Segment: Depth: " << BMP_GetDepth(subImgs[i]);
        std::cout << "Segment: Height: " << BMP_GetHeight(subImgs[i]);
        std::cout << "Segment: Width: " << BMP_GetWidth(subImgs[i]);
    }

    BMP* finalImage = BMP_Create(width, height, depth);

    uint32_t currOffsetH = 0;
    uint32_t currOffsetW = 0;

    uint32_t segHeight = std::floor(height / numSegments); 
    uint32_t segWidth  = width;

    for(int i = 0; i < subImgs.size(); i++) {

        std::cout << "Combining SubImage number: " << i << std::endl;

        for(int w = currOffsetW; w < segWidth; w++) {
            for(int h = currOffsetH; h < segHeight; h++) {
        
                uint8_t r;
                uint8_t g;
                uint8_t b;

                BMP_GetPixelRGB(subImgs[i], w, h, &r, &g, &b);
                BMP_SetPixelRGB(finalImage, w, h, r, g, b);
            }
        }

        currOffsetH += segHeight;
    }

    std::cout << "Expanding output buffer... " << std::endl;

    expandOutput(BMP_GetSizeInBytes(finalImage));

    char*  finalResult     = nullptr;
    size_t finalResultSize = 0;

    getOutput(finalResult, finalResultSize);

    std::cout << "Writing combined image... " << std::endl;
    
    BMP_WriteBuff(finalImage, (unsigned char*)finalResult, finalResultSize);
    BMP_Free(finalImage);

    FILE* f = fopen("after_combine.bmp", "wb");
    fwrite((unsigned char*)finalResult, sizeof(char), finalResultSize, f);
    fclose(f);


    for(int i = 0; i < subImgs.size(); i++) {

        BMP_Free(subImgs[i]);
    }
}

__stdcall void segmentData(int numNodes,
                            std::function<void(char*&, size_t&)>        getInput,
                            std::function<void(char*&, size_t&, int)>  getSegment,
                            std::function<void(size_t, int)>            expandSegment,
                            std::function<void(char*, size_t, int)>   writeSegment) {

    char*  input     = nullptr;
    size_t inputSize = 0;

    getInput(input, inputSize);

    std::cout << "Segmentor.segmentData: Size of input buffer: " << inputSize << std::endl;

    BMP* originalImg = BMP_ReadBuff((unsigned char*)input, inputSize);

    if(!originalImg) {

        std::cout << "Segmentor.segmentData: Failed to read image: " << BMP_GetErrorDescription() << std::endl;

        return;
    }

    uint32_t depth  = BMP_GetDepth(originalImg);  
    uint32_t width  = BMP_GetWidth(originalImg);
    uint32_t height = BMP_GetHeight(originalImg);

    uint32_t segWidth;
    uint32_t segHeight;

    if(numNodes == 0) {

        segWidth  = width;
        segHeight = height; 

    } else {

        //segWidth  = std::ceil(width / numNodes);
        segHeight = std::floor(height / numNodes);        

    }

    segWidth = width;

    size_t currOffsetH = 0;
    size_t currOffsetW = 0;

    std::cout << "Segmentor: Image Width: " << width << std::endl;
    std::cout << "Segmentor: Image Height: " << height << std::endl;
    std::cout << "Segmentor: Image Seg Width: " << segWidth << std::endl;
    std::cout << "Segmentor: Image Seg Height: " << segHeight << std::endl;


    for(int i = 0; i < numNodes; i++) {

        BMP* newImg = BMP_Create(segWidth, segHeight, depth);

        for(int w = currOffsetW; w < segWidth; w++) {
            for(int h = currOffsetH; h < segHeight; h++) {

                uint8_t r;
                uint8_t g;
                uint8_t b;

                BMP_GetPixelRGB(originalImg, w, h, &r, &g, &b);
                BMP_SetPixelRGB(newImg, w, h, r, g, b);
            }
        }

        currOffsetH += segHeight;

        std::cout << "Segmentor.seg: size of image out: " << BMP_GetSizeInBytes(newImg) << std::endl;

        //write image segment to new segment buffer! 

        expandSegment(BMP_GetSizeInBytes(newImg), i);

        char*  seg     = nullptr;
        size_t segSize = 0;

        getSegment(seg, segSize, i);

        BMP_WriteBuff(newImg, (unsigned char*)seg, segSize);

        BMP_Free(newImg);
    }

    BMP_Free(originalImg);
}
