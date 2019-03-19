#include <stdio.h>
#include "example_dll.h"

__stdcall void run(char *s, size_t size, Buffer& result)
{
    printf("Counting....\n");

    unsigned long long magic = 42;

    result.write((char*)&magic, sizeof(magic));
}

__stdcall void combine(std::vector<Buffer>& results, Buffer& finalResult)
{
    unsigned long long sum = 0;

    for(int i = 0; i < results.size(); i++) {
            
        sum += *(unsigned long long*)results[i].buff.getBase();
        
    }

    finalResult.write((char*)&sum, sizeof(unsigned long long));
}