#pragma once

#include "Manager.h"
#include "Buffer.h"
#include "Decoder.h"
#include "Encoder.h"
#include "IDispatcher.h"
#include "Resource.h"
#include "TaskQueue.h"

#include "NetworkManager.h"
#include "JobManager.h"

class NodeManager : public Manager<NodeManager> {
private:

    NetworkManager& m_netMan;
    JobManager&     m_jobMan;

public:
    NodeManager(IDispatcher& dispatcher, TaskQueue& queue, NetworkManager& netMan, JobManager& jobMan) 
		: Manager(dispatcher, queue, "node_manager"),
        m_netMan(netMan),
        m_jobMan(jobMan)
    {

    }

    void execute() {

        std::vector<Resource> res;

        getResources(5, res);

        for(int i = 0; i < res.size(); i++) {

            std::string incomingMsg = std::string(res[i].buff.getBase());

            std::vector<std::string> tokens = split(incomingMsg, '-');

            std::cout << "NodeManager: incoming msg: " << incomingMsg << std::endl;

            std::string msg = "";

            for(int j = 0; j < tokens.size(); j++) {

                std::cout << "NodeManager: incoming token: " << tokens[j] << std::endl;
            }

            if(tokens[0] == "get_active_nodes") {
                
                std::cout << "NodeManager: Processing status request." << std::endl; 

                auto& nodes = m_netMan.getActiveNodes();

                msg = "active_node_list-";

                for(int j = 0; j < nodes.size(); j++) {

                    msg += nodes[j];

                    if(j != nodes.size() - 1) {

                        msg += "-";
                    }
                }
            }

            if(tokens[0] == "create_job") {

                m_jobMan.createJob(tokens[1], tokens[2], tokens[3], m_netMan.getActiveNodes());
            }

            res[i].buff.clear();
            res[i].buff.write((char*)msg.c_str(), msg.size());


            std::cout << "NodeManager: status msg: " << msg << std::endl; 
            

            res[i].destManager = "net_manager";
        }

        putResources(res);

    }
};