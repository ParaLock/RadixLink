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

        auto writeBack = [this](Resource& res, WebMsgParser& parser) {

                std::string msg = parser.getMessage();

                res.buff.clear();
                res.buff.write((char*)msg.c_str(), msg.size());

                std::cout << "NodeManager: status msg: " << msg << std::endl; 
                
                res.destManager = "net_manager";
                
                std::vector<Resource> temp = {res};
                
                putResources(temp, "monitor");    
            
        };

        std::vector<Resource> res;

        getResources(5, res, "monitor");

        for(int i = 0; i < res.size(); i++) {

            WebMsgParser parser;
            parser.parse(res[i].buff);

            std::string op = parser.getScaler("op");

            std::string newOp = "result";
            std::string status = "none";
            std::string currJob = "none";


            std::cout << "NodeManager: incoming msg: " << op << std::endl;

            if(op == "get_active_nodes") {
            
                auto& nodes = m_netMan.getActiveNodes();

                parser.reset();

                newOp = "active_node_list";
                parser.encode("nodes", nodes);
            }

            if(op == "create_job") {
                
                std::string codeFn = parser.getScaler("codeFn");
                std::string dataFn = parser.getScaler("dataFn");
                std::string jobName = parser.getScaler("jobName");

                std::vector<std::string> activeNodes = parser.get("contrib_nodes");

                int jobID = -1;

                if(!m_jobMan.createJob(codeFn, dataFn, jobName, activeNodes, jobID)) {

                    status = "failed";
                } else {

                    status = "success";

                    currJob = std::to_string(jobID);

                }


            }

            if(op == "get_node_state") {

                std::string writingToo = "";
                std::string readingFrom = "";

                m_stateReg.getState<std::string>("reading_from", readingFrom);
                m_stateReg.getState<std::string>("writing_too", writingToo);

                parser.reset();

                newOp = "node_state";
                parser.encode("read_state", readingFrom);
                parser.encode("write_state", writingToo);

            }

            if(op == "write_result") {
                
                std::string id = parser.getScaler("jobID");

                unsigned int jobID = std::stoul(id);

                if(!m_jobMan.writeJobResultToDisk(jobID)) {

                    status = "failed";

                } else {

                    status = "none";
                }

            }

            parser.encode("op", newOp);
            parser.encode("job", currJob);
            parser.encode("status", status);
            
            writeBack(res[i], parser);
        }
    }
};