#ifndef EXAMPLE_DLL_H
#define EXAMPLE_DLL_H

#include <string>

#include "Buffer.h"
#include "Utils.h"

#include <cmath>

#include "qdbmp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_EXAMPLE_DLL
#define EXAMPLE_DLL __declspec(dllexport)
#else
#define EXAMPLE_DLL __declspec(dllimport)
#endif

void __stdcall EXAMPLE_DLL run(char *s, size_t size, Buffer& buff);
void __stdcall EXAMPLE_DLL combine(std::vector<Buffer*>& results, Buffer& finalResult);
void __stdcall EXAMPLE_DLL segmentData(int numNodes, Buffer& data, std::vector<Buffer>& segmentsOut);


int EXAMPLE_DLL Double(int x);

#ifdef __cplusplus
}
#endif


#endif  // EXAMPLE_DLL_H