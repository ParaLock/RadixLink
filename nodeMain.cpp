#include "Decoder.h"
#include "Encoder.h"
#include "Dispatcher.h"

#include "NetworkManager.h"
#include "JobManager.h"

#include <iostream>

int main(int argc, char **argv) {

    Encoder encoder;
    Decoder decoder;

    decoder.registerHandler(RESOURCE_TYPE_CODE, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Code section detected!" << std::endl;
        
        resource.codeFn = std::to_string(resource.jobID) + std::string(".dll");
        
        std::ofstream binFile(resource.codeFn, std::ios::out | std::ios::binary);
        
        
        if (binFile.is_open()) {
            
            binFile.write(payload, header->payloadSize);
        }

        resource.destManager = typeid(JobManager).name();
    });

    decoder.registerHandler(RESOURCE_TYPE_JOB, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Job section detected!" << std::endl;
        
        resource.info = *(JobInfo*)payload;

        resource.destManager = typeid(JobManager).name();	
    });

    decoder.registerHandler(RESOURCE_TYPE_DATA, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Data section detected!" << "payload size: " << header->payloadSize << std::endl;
    
        resource.buff.write(payload, header->payloadSize);

        resource.destManager = typeid(JobManager).name();
    });

    encoder.registerHandler(RESOURCE_TYPE_CODE, [](Buffer& buff, Resource& resource) {

        EncoderHeader header;
        header.type        = resource.type;
        header.payloadSize = resource.buff.getSize();
        header.jobID       = resource.jobID;

        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write(resource.buff.getBase(), header.payloadSize);

        resource.destManager = typeid(JobManager).name();
    });

    encoder.registerHandler(RESOURCE_TYPE_JOB, [](Buffer& buff, Resource& resource) {

        EncoderHeader header;
        header.type        = resource.type;
        header.payloadSize = resource.buff.getSize();
        header.jobID       = resource.jobID;

        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write((char*)&resource.info, sizeof(decltype(resource.info)));

        resource.destManager = typeid(JobManager).name();	
    });

    encoder.registerHandler(RESOURCE_TYPE_DATA, [](Buffer& buff, Resource& resource) {

        EncoderHeader header;
        header.type        = resource.type;
        header.payloadSize = resource.buff.getSize();
        header.jobID       = resource.jobID;

        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write((char*)&resource.info, sizeof(decltype(resource.info)));


        resource.destManager = typeid(JobManager).name();
    });

    Dispatcher dispatcher;

    NetworkManager netMan(dispatcher, decoder, encoder);
    JobManager     jobMan(dispatcher);



    auto myThread = std::thread([&netMan, &jobMan]() {
        netMan.execute();
        jobMan.execute();
    });

    Resource a;
    Resource b;
    Resource c;
    Resource d;

    char* data = "hello world!";

    JobInfo info;
    info.jobName[0] = 'h';
    info.jobName[0] = 'e';
    info.jobName[0] = 'l';
    info.jobName[0] = 'l';
    info.jobName[0] = 'o';
    info.jobName[0] = '\0';

    info.preReqs[0] = RESOURCE_TYPE_CODE;
    info.preReqs[1] = RESOURCE_TYPE_DATA;

    a.type          = RESOURCE_TYPE_JOB;
    a.jobID         = 0;
    a.info          = info;
    a.destManager   = typeid(JobManager).name();

    b.type          = RESOURCE_TYPE_CODE;
    b.jobID         = 0;
    b.codeFn        = std::to_string(b.jobID) + std::string(".dll");
    b.destManager   = typeid(JobManager).name();

    c.type          = RESOURCE_TYPE_DATA;
    c.jobID         = 0;
    c.destManager   = typeid(JobManager).name();

    c.buff.write(data, (int)strlen(data));

    std::vector<Resource> resources;
    
    resources.push_back(c);
    resources.push_back(b);
    resources.push_back(a);

    dispatcher.dispatch(resources);

    while(true) {
    }

    //netMan.start();
    //jobMan.start();

   return 0;
}