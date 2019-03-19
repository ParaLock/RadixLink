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

__stdcall void combine(std::vector<Buffer>& results, Buffer& finalResult)
{
    unsigned long long sum = 0;

    for(int i = 0; i < results.size(); i++) {
            
        sum += *(unsigned long long*)results[i].getBase();
        
    }

    finalResult.write((char*)&sum, sizeof(unsigned long long));
}