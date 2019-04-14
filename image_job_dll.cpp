#include <stdio.h>
#include <cmath>
#include "example_dll.h"

extern "C" {
    #include "qdbmp.c"
}

__stdcall void run(char *s, size_t size, Buffer& result)
{
    BMP* img = BMP_ReadBuff((unsigned char*)s, size);
    
    if(!img) {

        std::cout << "Segmentor.Run: Failed to read image: " << BMP_GetErrorDescription() << std::endl;
    }

    FILE* f = fopen("after.bmp", "wb");
    fwrite(s, sizeof(char), size, f);
    fclose(f);

    uint32_t w = 0, h = 0;

    h = BMP_GetHeight(img);
    w = BMP_GetWidth(img);
    
    
    
    
   
    #define filterWidth 5
    #define filterHeight 5

    double filter[filterHeight][filterWidth] =
    {
    -1,  0,  0,  0,  0,
    0, -2,  0,  0,  0,
    0,  0,  6,  0,  0,
    0,  0,  0, -2,  0,
    0,  0,  0,  0, -1,
    };

    double factor = 1.0;
    double bias = 0.0;

        
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

        BMP_SetPixelRGB(img, x, y, 
                                    std::min(std::max(int(factor * red + bias), 0), 255), 
                                    std::min(std::max(int(factor * green + bias), 0), 255), 
                                    std::min(std::max(int(factor * blue + bias), 0), 255));
    }

    result.resize(size);

    BMP_WriteBuff(img, (unsigned char*)result.getBase(), result.getSize());

    f = fopen("after_effect.bmp", "wb");
    fwrite((unsigned char*)result.getBase(), sizeof(char), result.getSize(), f);
    fclose(f);

    BMP_Free(img);
}

__stdcall void combine(std::vector<Buffer*>& results, Buffer& finalResult)
{
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t depth = 0;

    std::vector<BMP*> subImgs;

    for(int i = 0; i < results.size(); i++) {

        subImgs.push_back(BMP_ReadBuff((unsigned char*)results[i]->getBase(), results[i]->getSize())); 
    }

    for(int i = 0; i < results.size(); i++) {

        depth = BMP_GetDepth(subImgs[i]);

        height += BMP_GetHeight(subImgs[i]);
        width += BMP_GetWidth(subImgs[i]);
    }

    BMP* finalImage = BMP_Create(width, height, depth);

    uint32_t currOffsetH = 0;
    uint32_t currOffsetW = 0;

    uint32_t segHeight = std::ceil(height / results.size()); 
    uint32_t segWidth  = std::ceil(width / results.size());

    for(int i = 0; i < subImgs.size(); i++) {

        for(int w = currOffsetW; w < segWidth; w++) {
            for(int h = currOffsetH; h < segHeight; h++) {
        
                uint8_t r;
                uint8_t g;
                uint8_t b;

                BMP_GetPixelRGB(subImgs[i], w, h, &r, &g, &b);
                BMP_SetPixelRGB(finalImage, w, h, r, g, b);
            }
        }

        currOffsetW += segWidth;
        currOffsetH += segHeight;
    }

    finalResult.resize(BMP_GetSizeInBytes(finalImage));

    BMP_WriteBuff(finalImage, (unsigned char*)finalResult.getBase(), finalResult.getSize());
    BMP_Free(finalImage);

    for(int i = 0; i < subImgs.size(); i++) {

        BMP_Free(subImgs[i]);
    }
}

__stdcall void segmentData(int numNodes, Buffer& input, std::vector<Buffer>& segments) {

    BMP* originalImg = BMP_ReadBuff((unsigned char*)input.getBase(), input.getSize());

    if(!originalImg) {

        std::cout << "Segmentor.segmentData: Failed to read image: " << BMP_GetErrorDescription() << std::endl;
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

        segWidth  = std::ceil(width / numNodes);
        segHeight = std::ceil(height / numNodes);        

    }

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
        currOffsetW += segWidth;

        std::cout << "Segmentor.seg: size of image out: " << BMP_GetSizeInBytes(newImg) << std::endl;

        //write image segment to new segment buffer! 
        segments.push_back(Buffer(BMP_GetSizeInBytes(newImg)));

        auto& buff = segments.back();

        BMP_WriteBuff(newImg, (unsigned char*)buff.getBase(), buff.getSize());

        FILE* f = fopen("before.bmp", "wb");
        fwrite(buff.getBase(), sizeof(uint8_t), buff.getSize(), f);
        fclose(f);

        BMP_Free(newImg);
    }

    BMP_Free(originalImg);
}
