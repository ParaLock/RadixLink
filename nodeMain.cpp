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

    // Resource a;
    // Resource b;
    // Resource c;
    // Resource d;

    // char* data = "hello world!";

    // JobInfo info;
    // info.jobName[0] = 'h';
    // info.jobName[1] = 'e';
    // info.jobName[2] = 'l';
    // info.jobName[3] = 'l';
    // info.jobName[4] = 'o';
    // info.jobName[5] = '\0';

    // info.preReqs[0] = RESOURCE_TYPE_CODE;
    // info.preReqs[1] = RESOURCE_TYPE_DATA;

    // a.type          = RESOURCE_TYPE_JOB;
    // a.jobID         = 0;
    // a.info          = info;
    // a.destManager   = typeid(JobManager).name();

    // b.type          = RESOURCE_TYPE_CODE;
    // b.jobID         = 0;
    // b.codeFn        = std::to_string(b.jobID) + std::string(".dll");
    // b.destManager   = typeid(JobManager).name();

    // c.type          = RESOURCE_TYPE_DATA;
    // c.jobID         = 0;
    // c.destManager   = typeid(JobManager).name();

    // c.buff.write(data, (int)strlen(data));

    // std::vector<Resource> resources;
    
    // resources.push_back(c);
    // resources.push_back(b);
    // resources.push_back(a);

    //dispatcher.dispatch(resources);

    // std::map<int, std::function<void(std::vector<Resource>&)>> primary_actions;

    // primary_actions.insert({3, [&encoder](std::vector<Resource>& resources) {

    //     int         jobID;
    //     std::string codeFn;
    //     std::string jobName;
    //     std::string data;

    //     std::cout << "job id: ";
    //     std::cin >> jobID;

    //     std::cout << "code filename(max length 20 char): ";
    //     std::cin >> codeFn;

    //     std::cout << "job name(max length 20 char): ";
    //     std::cin >> jobName;

    //     std::cout << "data: ";
    //     std::cin >> data;



    // }});

    if(*argv[1] == 'S') {
        
        std::cout << "Creating Server: " << std::endl;

        netMan.createServer(DEFAULT_PORT);

        std::cout << "When client is active press a number" << std::endl;
        int test = 0;
        std::cin >> test;

        jobMan.createJob("example_dll.dll", "data.dat", "hello", "localhost");
    
    } else if(*argv[1] == 'C') {
        
        std::cout << "Creating Client: " << std::endl;


        netMan.connectToNode("localhost", DEFAULT_PORT);

    }

    int num = 0;
    std::cin >> num;

        // std::cout << "1) Enable This Node" << std::endl;
        // std::cout << "2) Connect to Node" << std::endl;
        // std::cout << "3) Create Job" << std::endl;
        // std::cout << "4) See Job results" << std::endl;
        

    
    
    WSACleanup();
    //netMan.start();
    //jobMan.start();

   return 0;
}