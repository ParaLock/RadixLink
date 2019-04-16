#ifndef EXAMPLE_DLL_H
#define EXAMPLE_DLL_H

#include <string>

#include "Utils.h"
#include "Buffer.h"

#include <cmath>

#include "qdbmp.h"

#ifdef __cplusplus
extern "C" {
#endif



__declspec(dllexport) __stdcall void run(
                                                std::function<void(char*&, size_t&)>       getInput,
                                                std::function<void(size_t)>                expandOutput,
                                                std::function<void(char*, size_t)>        writeOutput,
                                                std::function<void(char*&, size_t&)>      getOutput
                                        );


__declspec(dllexport) __stdcall void combine(
                        int                                        numSegments,
                        std::function<void(char*&, size_t&, int)>  getSegment,
                        std::function<void(char*&, size_t&)>       getOutput,
                        std::function<void(size_t)>                expandOutput,
                        std::function<void(char*, size_t)>         writeOutput
                    );

__declspec(dllexport) __stdcall void segmentData(   
                                                    int                                       numNodes,
                                                    std::function<void(char*&, size_t&)>        getInput,
                                                    std::function<void(char*&, size_t&, int)>  getSegment,
                                                    std::function<void(size_t, int)>            expandSegment,
                                                    std::function<void(char*, size_t, int)>   writeSegment
                                                );

int __declspec(dllexport) Double(int x);

#ifdef __cplusplus
}
#endif


#endif  // EXAMPLE_DLL_H