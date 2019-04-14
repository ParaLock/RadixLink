#include <stdio.h>
#include "example_dll.h"

__stdcall void run(char *s, size_t size, Buffer& result)
{
    printf("Counting....\n");

    unsigned long long start = *(unsigned long long*)s;
    unsigned long long end   = *(unsigned long long*)(s + sizeof(unsigned long long));

    printf("start: %d\n", start);
    printf("end: %d\n", end);

    unsigned long long sum = start;

    for(int i = start; i < end; i++) {

        sum += i;
    }

    printf("sum: %d\n", sum);

    result.write((char*)&sum, sizeof(sum));
}

__stdcall void combine(std::vector<Buffer*>& results, Buffer& finalResult)
{
    unsigned long long sum = 0;

    for(int i = 0; i < results.size(); i++) {
            
        sum += *(unsigned long long*)results[i]->getBase();
        
    }

    finalResult.write((char*)&sum, sizeof(unsigned long long));
}

__stdcall void segmentData(int numNodes, Buffer& input, std::vector<Buffer>& segments) {

    std::string str = std::string(input.getBase(), input.getSize());

    std::cout << "Segmentor: input: " << str << std::endl;

    std::vector<std::string> nums = split(str, '-');

    for(int i = 0; i < nums.size(); i++) {

        std::cout << "Segmenter: segment value: " << std::stoull(nums[i]) << std::endl;
    }

    int count = 0;

    for(int i = 0; i < nums.size() / 2; i++) {

        segments.push_back(Buffer());
    }

    for(int i = 0; i < segments.size(); i++) {

        for(int j = 0; j < 2; j++) {

            unsigned long long data = std::stoull(nums[count]);
            auto& buff = segments[i];
            buff.write((char*)&data, sizeof(unsigned long long));

            count++;
        }
    }

}
