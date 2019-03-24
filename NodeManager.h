#pragma once

#include "Manager.h"
#include "Buffer.h"
#include "Decoder.h"
#include "Encoder.h"
#include "IDispatcher.h"
#include "Resource.h"
#include "TaskQueue.h"

#include "NetworkManager.h"

class NodeManager : public Manager<NodeManager> {
private:

    NetworkManager& m_netMan;


public:
    NodeManager(IDispatcher& dispatcher, TaskQueue& queue, NetworkManager& netMan) 
		: Manager(dispatcher, queue, "node_manager"),
        m_netMan(netMan)
    {

    }

    void execute() {

        std::vector<Resource> res;

        getResources(5, res);

        for(int i = 0; i < res.size(); i++) {

            std::string incomingMsg = std::string(res[i].buff.getBase());

            std::vector<std::string> tokens = split(incomingMsg, '-');

            std::cout << "NodeManager: incoming msg: " << incomingMsg << std::endl;

            if(tokens[0] == "get_active_nodes") {

                auto& nodes = m_netMan.getActiveNodes();

                std::string msg = "active_node_list-";

                for(int j = 0; j < nodes.size(); j++) {

                    msg += nodes[i];

                    if(j != nodes.size() - 1) {

                        msg += "-";
                    }
                }

                std::cout << "NodeManager: status msg: " << msg << std::endl; 
            }


            

            res[i].destManager = "net_manager";
        }

        putResources(res);

    }
};