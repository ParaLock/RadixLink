#include "Decoder.h"
#include "Encoder.h"
#include "Dispatcher.h"

#include "NetworkManager.h"
#include "JobManager.h"
#include "ConfigLoader.h"
#include "TaskExecutor.h"
#include "NodeManager.h"
#include "StateRegistry.h"

#include "jobtypes/DllJob.h"

#include "ObjectPool.h"

#include <iostream>

#define JOB_TYPE_DLL 0

struct Test : ObjectPool::Pooled {

    int a;
    int b;

    void reset() {

        a = -2;
        b = -2;
    }

};

int main(int argc, char **argv) {



    // ObjectPool::Pool<Test> pool(16);

    // Test* t1 = pool.getItem();
    // t1->a = 3;
    // t1->b = 4;

    // Test* t2 = pool.getItem();
    // pool.freeItem(t2);

    // t2 = pool.getItem();

    // std::cout << "t2: " << t2->a << " " << t2->b << std::endl;

    // Buffer testBuff;

    // for(int i = 0; i < 2047; i++) {

    //     char d[1];
    //     d[0] = 'A';
    //     testBuff.write(&d[0], 1);
    // }

    // char temp[9000];

    WSADATA wsaData;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    srand(time(NULL));

    std::cout << sizeof(unsigned int);
 
    Encoder encoder;
    Decoder decoder;

    decoder.registerHandler(RESOURCE_TYPE_CODE, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Code section detected! " << "payload size: " << header->payloadSize << std::endl;
        
        resource.codeFn = std::to_string(resource.jobID) + std::string(".code");
        
        std::ofstream binFile(resource.codeFn, std::ios::out | std::ios::binary);
        
        
        if (binFile.is_open()) {
            
            binFile.write(payload, header->payloadSize);
        }
        resource.jobType = header->jobType;
        resource.destManager = "job_manager";
    });

    decoder.registerHandler(RESOURCE_TYPE_JOB, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Job section detected!" << " payload size: " << header->payloadSize << std::endl;
        
        resource.info = *(JobInfo*)payload;
        resource.jobType = header->jobType;
        resource.destManager = "job_manager";
    });

    decoder.registerHandler(RESOURCE_TYPE_DATA, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Data section detected!" << " payload size: " << header->payloadSize << std::endl;

        resource.buff.write(payload, header->payloadSize);
        resource.jobType = header->jobType;
        resource.destManager = "job_manager";
    });

    decoder.registerHandler(RESOURCE_TYPE_RESULT, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Result section detected!" << " payload size: " << header->payloadSize << std::endl;
        
        char* temp = payload;

        for(int i = 0; i < sizeof(resource.target); i++) {
  
            resource.target[i] = (char)payload[i];
            temp++;
        }

        resource.order = *(unsigned int*)temp;
        temp += sizeof(unsigned int);

        resource.buff.write(temp, header->payloadSize - sizeof(resource.target));
        resource.jobType = header->jobType;
        resource.destManager = "job_manager";
    });

    decoder.registerHandler(RESOURCE_TYPE_STATUS, [](EncoderHeader* header, char* payload, Resource& resource) {

        std::cout << "Decoder: Status section detected!" << " payload size: " << header->payloadSize << std::endl;
        
        
        resource.buff.write(payload, header->payloadSize);
        resource.jobType = header->jobType;

        resource.target[0] = '1';
        resource.target[1] = '2';
        resource.target[2] = '7';
        resource.target[3] = '.';
        resource.target[4] = '0';
        resource.target[5] = '.';
        resource.target[6] = '0';
        resource.target[7] = '.';
        resource.target[8] = '1';
        resource.target[9] = '\0';

        resource.destManager = "node_manager";
    });

    encoder.registerHandler(RESOURCE_TYPE_CODE, [](Buffer& buff, Resource& resource) {

        std::cout << "Encoder: Code section detected!" << " payload size: " << resource.buff.getSize() << std::endl;

        EncoderHeader header;
        header.type        = resource.type;
        header.payloadSize = resource.buff.getSize();
        header.jobID       = resource.jobID;
        header.jobType     = resource.jobType;


        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write(resource.buff.getBase(), header.payloadSize);

    });

    encoder.registerHandler(RESOURCE_TYPE_JOB, [](Buffer& buff, Resource& resource) {

        std::cout << "Encoder: Job section detected!" << " payload size: " << sizeof(decltype(resource.info)) << std::endl;

        EncoderHeader header;
        header.type        = resource.type;
        header.payloadSize = sizeof(decltype(resource.info));
        header.jobID       = resource.jobID;
        header.jobType     = resource.jobType;

        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write((char*)&resource.info, sizeof(decltype(resource.info)));
    });

    encoder.registerHandler(RESOURCE_TYPE_DATA, [](Buffer& buff, Resource& resource) {

        std::cout << "Encoder: Data section detected!" << " payload size: " << resource.buff.getSize() << std::endl;


        EncoderHeader header;
        header.type        = resource.type;
        header.payloadSize = resource.buff.getSize();
        header.jobID       = resource.jobID;
        header.jobType     = resource.jobType;

        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write(resource.buff.getBase(), header.payloadSize);

        resource.destManager = "job_manager";
    });

    encoder.registerHandler(RESOURCE_TYPE_RESULT, [](Buffer& buff, Resource& resource) {


        std::cout << "Encoder: Result section detected!" << " payload size: " << resource.buff.getSize() + sizeof(resource.target) + sizeof(resource.order) << std::endl;
        char test[5];
        test[0] = 'T';
        test[1] = 'E';
        test[2] = 'S';
        test[3] = 'T';
        test[4] = '7';

        resource.buff.write(test, sizeof(test));

        EncoderHeader header;
        header.type         = resource.type;
        header.payloadSize  = resource.buff.getSize() + sizeof(resource.target) + sizeof(resource.order);
        header.jobID        = resource.jobID;
        header.jobType     = resource.jobType;


        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write((char*)&resource.target, sizeof(resource.target));
        buff.write((char*)&resource.order, sizeof(resource.order));
        buff.write(resource.buff.getBase(), resource.buff.getSize());


        resource.destManager = "job_manager";

    });

    encoder.registerHandler(RESOURCE_TYPE_STATUS, [](Buffer& buff, Resource& resource) {

        std::cout << "Encoder: Status section detected!" << std::endl;

        EncoderHeader header;
        header.type         = resource.type;
        header.payloadSize  = resource.buff.getSize();
        header.jobID        = resource.jobID;
        header.jobType     = resource.jobType;


        if(resource.buff.getSize() < 5) {

            std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BAD WEB MSG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        }

        buff.clear();

        buff.write((char*)&header, sizeof(EncoderHeader));
        buff.write(resource.buff.getBase(), resource.buff.getSize());

    });

    StateRegistry stateReg(50);
    Dispatcher    dispatcher;
    TaskExecutor     TaskExecutor;

    TaskExecutor.createQueue("test_queue", 4);

    TaskExecutor.addTask(Task("test_queue", []() {

        std::cout << "TEST COMPLETED1!" << std::endl;

    }, []() {} ));

    TaskExecutor.addTask(Task("test_queue", []() {

        Sleep(3000);

        std::cout << "TEST COMPLETED2!" << std::endl;

    }, []() {} ));

    TaskExecutor.addTask(Task("test_queue", []() {

        Sleep(100);

        std::cout << "TEST COMPLETED3!" << std::endl;

    }, []() {} ));

    TaskExecutor.addTask(Task("test_queue", []() {

        Sleep(50);

        std::cout << "TEST COMPLETED4!" << std::endl;

    }, []() {} ));

    TaskExecutor.addTask(Task("test_queue", []() {

        std::cout << "TEST COMPLETED5!" << std::endl;

    }, []() {} ));

    NetworkManager netMan(dispatcher, TaskExecutor, stateReg, decoder, encoder);
    JobManager     jobMan(dispatcher, TaskExecutor, netMan, stateReg);
    NodeManager    nodeMan(dispatcher, TaskExecutor, stateReg, netMan, jobMan);

    jobMan.registerJobType([]() {

        return new DllJob();
        
    }, JOB_TYPE_DLL);

    std::map<int, std::function<void()>> primary_actions;

    primary_actions.insert({1, [&jobMan, &netMan]() {

        std::string codeFn;
        std::string jobName;
        std::string data;

        std::cout << "code filename(max length 20 char): ";
        std::cin >> codeFn;

        std::cout << "job name(max length 20 char): ";
        std::cin >> jobName;

        std::cout << "data file: ";
        std::cin >> data;

        int jobID = -1;

        if(!jobMan.createJob(codeFn, data, JOB_TYPE_DLL, netMan.getActiveNodes(), jobID)) {

            std::cout << "App: Job creation failed!" << std::endl;
        }
    }});

    primary_actions.insert({4, [&jobMan]{
        
        int jobID = -1;

        std::cout << "Please enter job id: ";
        std::cin >> jobID;

        jobMan.printJobResults(jobID);
    }});

    primary_actions.insert({5, [&jobMan, &netMan]{

        std::string fn;

        std::cout << "Config file name: ";
        std::cin >> fn;


        ConfigLoader conf;

        if(conf.loadFile(fn)) {

            std::vector<std::string> ipList;
            conf.getList("IPs", ipList);

            std::string policy;
            conf.getScaler("policy", policy);

            netMan.setDebugMode(policy);

            //First ip address is always "this" node...
            netMan.createServer(ipList[0], DEFAULT_PORT);
        
            for(int i = 1; i < ipList.size(); i++) {

                if(!netMan.connectToNode(ipList[i].c_str(), DEFAULT_PORT)) {
                    std::cout << "Adding pending connection..." << std::endl;
                }
            }

        }
    }});

    // std::cout << "SIZE_OF SIZE_T SIZE: " << sizeof(size_t) << std::endl;
    // std::cout << "SIZE_OF UINT SIZE: " << sizeof(unsigned int) << std::endl;
    // std::cout << "SIZE_OF INT SIZE: " << sizeof(int) << std::endl;

    netMan.start();
    jobMan.start();
    nodeMan.start();

    primary_actions.insert({9, [&netMan]() {

        netMan.testRead();
    }});

    primary_actions.insert({10, [&netMan]() {

        netMan.testWrite();
    }});

    primary_actions.insert({6, []{}});

    int op = 0;

    while(op != 6) {
    
        std::cout << "1) Create Job" << std::endl;
        std::cout << "2) See Current Outgoing Jobs" << std::endl;
        std::cout << "3) See Current Incoming Jobs" << std::endl;
        std::cout << "4) View Job Result" << std::endl;
        std::cout << "5) Load Configuration" << std::endl;
        std::cout << "9) test read" << std::endl;
        std::cout << "10) test write" << std::endl;
        std::cout << "6) Exit" << std::endl;
    
        std::cin >> op;

        auto itr = primary_actions.find(op);

        if(itr != primary_actions.end()) {
           
            primary_actions.at(op)();

        }
    }

    netMan.stop();
    jobMan.stop();
    nodeMan.stop();
    
    TaskExecutor.stop();

    WSACleanup();

   return 0;
}