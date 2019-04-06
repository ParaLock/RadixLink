#pragma once

#include "Manager.h"
#include "Buffer.h"
#include "Decoder.h"
#include "Encoder.h"
#include "IDispatcher.h"
#include "Resource.h"
#include "TaskQueue.h"
#include "WebMsgParser.h"

#include "NetworkManager.h"
#include "JobManager.h"

class NodeManager : public Manager<NodeManager> {
private:

    NetworkManager& m_netMan;
    JobManager&     m_jobMan;

public:
    NodeManager(IDispatcher& dispatcher, TaskQueue& queue, StateRegistry& reg, NetworkManager& netMan, JobManager& jobMan) 
		: Manager(dispatcher, queue, reg, "node_manager", false),
        m_netMan(netMan),
        m_jobMan(jobMan)
    {

    }

    void execute() {

        std::vector<Resource> res;

        getResources(5, res, "monitor");

        for(int i = 0; i < res.size(); i++) {

            WebMsgParser parser;
            parser.parse(res[i].buff);

            //std::string op = parser.getScaler("op");

            //std::cout << "NodeManager: incoming msg: " << op << std::endl;

            // if(op == "get_active_nodes") {
                
            //     std::cout << "NodeManager: Processing status request." << std::endl; 

            //     auto& nodes = m_netMan.getActiveNodes();

            //     parser.encode("op", "active_node_list");
            //     parser.encode("nodes", nodes);
            // }

            // if(op == "create_job") {
                
            //     std::string codeFn = parser.getScaler("codeFn");
            //     std::string dataFn = parser.getScaler("dataFn");
            //     std::string jobName = parser.getScaler("jobName");

            //     m_jobMan.createJob(codeFn, dataFn, jobName, m_netMan.getActiveNodes());
            // }

            // if(op == "get_node_state") {

            //     auto boolToStr = [](bool b)
            //     {
            //         return b ? "true" : "false";
            //     };

            //     std::string writingToo = m_stateReg.getState<std::string>("writing_too");
            //     std::string readingFrom = m_stateReg.getState<std::string>("reading_from");

            //     parser.encode("read_state", readingFrom);
            //     parser.encode("write_state", writingToo);
            // }

            // std::string msg = parser.getMessage();

            // res[i].buff.clear();
            // res[i].buff.write((char*)msg.c_str(), msg.size());

            // std::cout << "NodeManager: status msg: " << msg << std::endl; 
            
            // res[i].destManager = "net_manager";
        }

        //putResources(res, "monitor");

    }
};