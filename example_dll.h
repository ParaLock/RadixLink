#ifndef EXAMPLE_DLL_H
#define EXAMPLE_DLL_H

#include "Buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_EXAMPLE_DLL
#define EXAMPLE_DLL __declspec(dllexport)
#else
#define EXAMPLE_DLL __declspec(dllimport)
#endif

void __stdcall EXAMPLE_DLL run(char *s, size_t size, Buffer& buff);
void __stdcall EXAMPLE_DLL combine(std::vector<Buffer>& results, Buffer& finalResult);

int EXAMPLE_DLL Double(int x);

#ifdef __cplusplus
}
#endif


#endif  // EXAMPLE_DLL_H