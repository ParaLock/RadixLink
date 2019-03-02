#include "Decoder.h"
#include "Encoder.h"
#include "Dispatcher.h"

#include <iostream>

int main() {

    Decoder decoder;

    decoder.registerHandler(0, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Code section detected!" << std::endl;
        
        resource.codeFn = std::to_string(resource.jobID) + std::string(".dll");
        
        std::ofstream binFile(resource.codeFn, std::ios::out | std::ios::binary);
        
        
        if (binFile.is_open()) {
            
            binFile.write(payload, header->payloadSize);
        }

        resource.destManager = "job_manager";
    
			
    });

    decoder.registerHandler(1, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Job section detected!" << std::endl;
        
        resource.info = *(JobInfo*)payload;

        resource.destManager = "job_manager";
			
    });

    decoder.registerHandler(2, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Data section detected!" << "payload size: " << header->payloadSize << std::endl;
    
        resource.buff.write(payload, header->payloadSize);

        resource.destManager = "job_manager";
    });

    Dispatcher dispatcher;

    NetworkManager netMan;
    JobManager     jobMan;


}