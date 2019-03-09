#include "Decoder.h"
#include "Encoder.h"
#include "Dispatcher.h"

#include "NetworkManager.h"
#include "JobManager.h"

#include <iostream>

int main(int argc, char **argv) {


    WSADATA wsaData;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    srand(time(NULL));
    Encoder encoder;
    Decoder decoder;

    decoder.registerHandler(RESOURCE_TYPE_CODE, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Code section detected! " << "payload size: " << header->payloadSize << std::endl;
        
        resource.codeFn = std::to_string(resource.jobID) + std::string(".dll");
        
        std::ofstream binFile(resource.codeFn, std::ios::out | std::ios::binary);
        
        
        if (binFile.is_open()) {
            
            binFile.write(payload, header->payloadSize);
        }

        resource.destManager = "job_manager";
    });

    decoder.registerHandler(RESOURCE_TYPE_JOB, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Job section detected!" << " payload size: " << header->payloadSize << std::endl;
        
        resource.info = *(JobInfo*)payload;
        
        resource.isOutgoing = false;
        resource.destManager = "job_manager";
    });

    decoder.registerHandler(RESOURCE_TYPE_DATA, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Data section detected!" << " payload size: " << header->payloadSize << std::endl;

        resource.buff.write(payload, header->payloadSize);

        resource.destManager = "job_manager";
    });

    encoder.registerHandler(RESOURCE_TYPE_CODE, [](Buffer& buff, Resource& resource) {

        std::cout << "Encoder: Code section detected!" << " payload size: " << resource.buff.getSize() << std::endl;

        EncoderHeader header;
        header.type        = resource.type;
        header.payloadSize = resource.buff.getSize();
        header.jobID       = resource.jobID;

        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write(resource.buff.getBase(), header.payloadSize);

        resource.destManager = "job_manager";
    });

    encoder.registerHandler(RESOURCE_TYPE_JOB, [](Buffer& buff, Resource& resource) {

        std::cout << "Encoder: Job section detected!" << " payload size: " << sizeof(decltype(resource.info)) << std::endl;

        EncoderHeader header;
        header.type        = resource.type;
        header.payloadSize = sizeof(decltype(resource.info));
        header.jobID       = resource.jobID;

        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write((char*)&resource.info, sizeof(decltype(resource.info)));

        resource.destManager = "job_manager";
    });

    encoder.registerHandler(RESOURCE_TYPE_DATA, [](Buffer& buff, Resource& resource) {

        std::cout << "Encoder: Data section detected!" << " payload size: " << resource.buff.getSize() << std::endl;


        EncoderHeader header;
        header.type        = resource.type;
        header.payloadSize = resource.buff.getSize();
        header.jobID       = resource.jobID;

        //if(resource.dataFn.size() > 0) {

        //} else {


            buff.write((char*)&header, sizeof(EncoderHeader));
            buff.write(resource.buff.getBase(), header.payloadSize);
        //}
        
        resource.destManager = "job_manager";
    });

    Dispatcher dispatcher;

    NetworkManager netMan(dispatcher, decoder, encoder);
    JobManager     jobMan(dispatcher);



    auto myThread = std::thread([&netMan, &jobMan]() {
        while(true) {
            netMan.execute();
            jobMan.execute();
        }
    });

    std::map<int, std::function<void()>> primary_actions;

    primary_actions.insert({3, [&jobMan]() {

        std::string codeFn;
        std::string jobName;
        std::string data;

        std::cout << "code filename(max length 20 char): ";
        std::cin >> codeFn;

        std::cout << "job name(max length 20 char): ";
        std::cin >> jobName;

        std::cout << "data filename: ";
        std::cin >> data;

        jobMan.createJob("example_dll.dll", "data.dat", "hello", "127.0.0.1");
    }});

    primary_actions.insert({2, [&netMan]() {

        std::cout << "Please enter node name: " << std::endl;
        std::string nodeName = ""; 

        std::cin >> nodeName;

        netMan.connectToNode("localhost", DEFAULT_PORT);

    }});

    primary_actions.insert({1, [&netMan]() {

        netMan.createServer(DEFAULT_PORT);

    }});

    int op = 0;

    while(op < 6) {
    
        std::cout << "1) Enable This Node" << std::endl;
        std::cout << "2) Connect to Node" << std::endl;
        std::cout << "3) Create Job" << std::endl;
        std::cout << "4) See Current Outgoing Jobs" << std::endl;
        std::cout << "5) See Current Incoming Jobs" << std::endl;
        std::cout << "6) Exit" << std::endl;
    
        std::cin >> op;

        primary_actions.at(op)();

    }

    WSACleanup();

   return 0;
}