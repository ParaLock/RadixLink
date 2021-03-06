#include <stdio.h>
#include "example_dll.h"




__stdcall void run(
                    std::function<void(char*&, size_t&)>       getInput,
                    std::function<void(size_t)>                expandOutput,
                    std::function<void(char*, size_t)>        writeOutput,
                    std::function<void(char*&, size_t&)>      getOutput)
{
    printf("Counting....\n");

    char* s = nullptr;
    size_t size = 0;

    getInput(s, size);

    //printf("Input (size, ptr): %s, %p....\n", size, s);

    printf("Got input\n");

    uint64_t start = *(uint64_t*)s;
    uint64_t end   = *(uint64_t*)(s + sizeof(uint64_t));

    printf("start: %d\n", start);
    printf("end: %d\n", end);

    uint64_t sum = start;

    for(int i = start; i < end; i++) {

        sum += i;
    }

    printf("sum: %d\n", sum);

    writeOutput((char*)&sum, sizeof(sum));
}

__stdcall void combine(
                        int                                        numSegments,
                        std::function<void(char*&, size_t&, int)>  getSegment,
                        std::function<void(char*&, size_t&)>       getOutput,
                        std::function<void(size_t)>                expandOutput,
                        std::function<void(char*, size_t)>         writeOutput
                    )
{
    uint64_t sum = 0;

    for(int i = 0; i < numSegments; i++) {
        
		size_t segSize;
		char*  segData;

		getSegment(segData, segSize, i);

        sum += *(uint64_t*)segData;
    }

    writeOutput((char*)&sum, sizeof(uint64_t));
}

__stdcall void segmentData(   
                            int                                       numNodes,
                            std::function<void(char*&, size_t&)>        getInput,
                            std::function<void(char*&, size_t&, int)>  getSegment,
                            std::function<void(size_t, int)>            expandSegment,
                            std::function<void(char*, size_t, int)>   writeSegment)
 {
    char* input = nullptr;
    size_t inputSize = 0;
    getInput(input, inputSize);

    std::string str = std::string(input, inputSize);

    std::cout << "Segmentor: input: " << str << std::endl;

    std::vector<std::string> nums = split(str, '-');

    for(int i = 0; i < nums.size(); i++) {

        std::cout << "Segmenter: segment value: " << std::stoull(nums[i]) << std::endl;
    }

    // uint64_t data = std::stoull(nums[0]);
	// writeSegment((char*)&data, sizeof(uint64_t), 0);
    // data = std::stoull(nums[1]);
	// writeSegment((char*)&data, sizeof(uint64_t), 0);
    // data = std::stoull(nums[2]);
	// writeSegment((char*)&data, sizeof(uint64_t), 1);
    // data = std::stoull(nums[3]);
	// writeSegment((char*)&data, sizeof(uint64_t), 1);

    int segCount = 0;
    int count    = 0;

    for(int i = 0; i < nums.size(); i++) {

        uint64_t data = std::stoull(nums[i]);
	    writeSegment((char*)&data, sizeof(uint64_t), segCount);

        count++;

        if(count == 2) {
            count = 0;
            segCount++;
        }
    }

}
