#include <stdio.h>
#include "example_dll.h"

__stdcall void hello(char *s, size_t size, Buffer& result)
{
        printf("Hello %s\n", s);

        int magic = 42;

        result.write((char*)&magic, sizeof(magic));
}
