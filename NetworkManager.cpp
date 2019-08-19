
#include "NetworkManager.h"

// bool NetworkManager::connectToNodes(const char* target, const char* port) {

//     std::string tempTarget = target;
//     std::string tempPort   = port;

//     //m_pendingConnections.push_back(std::make_pair(target, port));

// 	return true;
// }



bool NetworkManager::connectToNode(const char* target, const char* port) {

    std::string tempTarget = target;
    std::string tempPort   = port;

    m_workExecutor.addTask(Task(
			"net_connection",
            [tempTarget, tempPort, this]() {

                this->processConnection(tempTarget, tempPort);
            }
	));
    
	return true;
}


void initBuffer(Buffer& out_buff) {

    char testBuff[15]; 
    
    for(int i = 0; i < 15; i++) {

        if(i < 15)
            testBuff[i] = 'A';
        else
            testBuff[i] = 'B';
    }

    // char testBuff[10]; 

    // testBuff[0] = 'A';
    // testBuff[1] = 'B';
    // testBuff[2] = 'c';
    // testBuff[3] = 'd';
    // testBuff[4] = 'E';
    // testBuff[5] = 'F';
    // testBuff[6] = 'G';
    // testBuff[7] = 'H';
    // testBuff[8] = 'Q';
    // testBuff[9] = 'P';
    
    out_buff.write(testBuff, sizeof(testBuff));
}

void NetworkManager::processConnection(std::string target, std::string port) {


    while(true) {

        Connection connection;
        connection.ip = target;
        connection.port = port;

        SOCKET ConnectSocket = INVALID_SOCKET;
        struct addrinfo *result = NULL,
                        hints;
        int iResult;

        iResult = getaddrinfo(connection.ip.c_str(), connection.port.c_str(), NULL, &result);
        if ( iResult != 0 ) {

            printf("getaddrinfo failed with error: %d\n", iResult);
            return;
        }

        ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol); 

        if (ConnectSocket == INVALID_SOCKET) {

            printf("socket failed with error: %ld\n", WSAGetLastError());
            return;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, result->ai_addr, (int)result->ai_addrlen);

        if (iResult == SOCKET_ERROR) {

            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        if (iResult == SOCKET_ERROR) {

            printf("socket failed with error: %ld\n", WSAGetLastError());
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        std::cout << "NetworkManager: Connected to node: " << target << std::endl;

        connection.socket = ConnectSocket;

        m_activeConnections.insert(target);

        if(m_debugMode == "CLIENT") {

                if (m_ioGroup.addStream(target, connection.socket)) {

                    m_ioGroup.beginWrite(target, []() {

                    },
                        [this, target]() {
                        
                        std::cout << "STREAM WRITE: " << target << " BEGIN" << std::endl;


                        Buffer out_buff;

                        //initBuffer(out_buff);

                        std::vector<Resource> resources;

                        this->getResourcesByTarget(target, 5, resources, "primary");

                        for (int i = 0; i < resources.size(); i++) {

                            strcpy(resources[i].target, this->m_name.c_str());

                        }

                        this->m_encoder.run(out_buff, resources);

                        return out_buff;
                    });

                    m_ioGroup.beginRead(target, [this, target](Buffer& data) {

                        //std::cout << "STREAM READ: " << target << " BEGIN" << std::endl;

                        std::vector<Resource> resources;

                        this->m_decoder.run(data, resources);
                        this->putResources(resources, "primary");

                        //data.print(); 
                       // std::cout << "STREAM READ: " << target << " END" << std::endl;
                    },
                    []() {



                    });


                    m_ioGroup.activate();


                }

            }

        break;
    }

}

bool NetworkManager::disconnect(std::string nodeName) {
	
	// Connection con = m_connections.at(nodeName);
	
	// int iResult;
	// // shutdown the connection since no more data will be sent
    // iResult = shutdown(con.socket, SD_SEND);
    // if (iResult == SOCKET_ERROR) {
    //     printf("shutdown failed with error: %d\n", WSAGetLastError());
    //     closesocket(con.socket);
    //     return false;
    // }
	
	return true;
	
}

void NetworkManager::testRead() {
   

    std::cout << "Running  read test! __START" << std::endl;

    // testBuffInfo.len = sizeof(testBuff);
    // testBuffInfo.buf = testBuff;
    

    // int result = WSASend(
    //                             testSocket,
    //                             &testBuffInfo,
    //                             1,
    //                             (LPDWORD)&testSent,
    //                             0,
    //                             (OVERLAPPED*)&testOverlapped,
    //                             NULL
    //                             );

    // std::cout << "TEST Write End: Result: " << result << "Status: " << WSAGetLastError() << std::endl;


 m_ioGroup.triggerRead(testSocket);

    std::cout << "Running  read test! __END" << std::endl;
}

void NetworkManager::testWrite() {
   

    std::cout << "Running write test! __START" << std::endl;

    // testBuffInfo.len = sizeof(testBuff);
    // testBuffInfo.buf = testBuff;
    

    // int result = WSASend(
    //                             testSocket,
    //                             &testBuffInfo,
    //                             1,
    //                             (LPDWORD)&testSent,
    //                             0,
    //                             (OVERLAPPED*)&testOverlapped,
    //                             NULL
    //                             );

    // std::cout << "TEST Write End: Result: " << result << "Status: " << WSAGetLastError() << std::endl;

    for(int i = 0; i < 2; i++) {

        m_ioGroup.triggerWrite(testSocket);

    }

    std::cout << "Running write test! __END" << std::endl;
}


void NetworkManager::customShutdown() {

    m_monitorConnected = false;
    m_listening        = false;
    m_connecting       = false;
    m_serverCreated    = false;

    std::cout << "NetworkManager: Closing listener Socket!" << std::endl;
    //closesocket(m_listenSocket);

    //for (auto const& s : m_connections) {

        std::cout << "NetworkManager: Shutting down connection!" << std::endl;

        //closesocket(s.second.socket);
    //}
}

bool NetworkManager::createServer(std::string ip, const char* port) {


    struct addrinfo *result = NULL;
    struct addrinfo hints;
	
	int iResult;
	
	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(ip.c_str(), DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        return 1;
    }

    m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( m_listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(m_listenSocket);
        return 1;
    }

             
    iResult = listen(m_listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(m_listenSocket);
        return 1;
    }

    freeaddrinfo(result);
	
    m_name = ip;

    std::cout << "NetManager: Server created -> IP: " << ip << std::endl;

    m_serverCreated = true;

	return true;

}

	
void NetworkManager::acceptConnection() {
	
    while(m_listening) {

        if(m_serverCreated) {
            
            Connection con;

            SOCKADDR_IN client_info = {0};
            int addrsize            = sizeof(client_info);

            SOCKET ClientSocket = INVALID_SOCKET;
            
            // Accept a client socket
            ClientSocket = accept(m_listenSocket, (struct sockaddr*)&client_info, &addrsize);
            if (ClientSocket == INVALID_SOCKET) {

                printf("accept failed with error: %d\n", WSAGetLastError());

                closesocket(m_listenSocket);

                return;
            }
            
            std::string ip = std::string(inet_ntoa(client_info.sin_addr));

            con.socket = ClientSocket;
            con.ip     = ip;

            std::cout << "NetworkManager: Client " << ip << " connected" << std::endl;

            // if(name == "127.0.0.1") {

            //     m_monitorConnected = true;
                
            //     m_activeConnections.insert(ip);

            //     continue;
            // }

            testSocket = ip;
            std::string target = ip;

            m_activeConnections.insert(ip);

            if(m_debugMode == "SERVER") {

                if (m_ioGroup.addStream(target, con.socket)) {

                    m_ioGroup.beginWrite(target, []() {

                    },
                        [this, target]() {
                        
                       // std::cout << "STREAM WRITE: " << target << " BEGIN" << std::endl;


                        Buffer out_buff;

                        
                       // initBuffer(out_buff);

                        std::vector<Resource> resources;

                        this->getResourcesByTarget(target, 5, resources, "primary");

                        for (int i = 0; i < resources.size(); i++) {

                            strcpy(resources[i].target, this->m_name.c_str());

                        }

                        this->m_encoder.run(out_buff, resources);

                        //std::cout << "STREAM WRITE: " << target << " END" << std::endl;
                        
                        return out_buff;
                    });

                    m_ioGroup.beginRead(target, [this, target](Buffer& data) {

                        //std::cout << "STREAM READ: " << target << " BEGIN" << std::endl;

                        std::vector<Resource> resources;
                        

                        //data.print(); 
                        this->m_decoder.run(data, resources);
                        this->putResources(resources, "primary");

                        //std::cout << "STREAM READ: " << target << " END" << std::endl;
                    },
                    []() {



                    });


                    m_ioGroup.activate();
                }

            }

            std::cout << "Client: " << ip << " has connected" << std::endl;
        }
    }
}

void NetworkManager::monitoringLoop() {

}

std::vector<std::string> NetworkManager::getActiveNodes() {

    std::vector<std::string> activeConnections;

    for(auto itr = m_activeConnections.begin(); itr != m_activeConnections.end(); itr++) {

        activeConnections.push_back(*itr);
    }

    return activeConnections;
}

void NetworkManager::execute() {
    
    for(auto itr = m_activeConnections.begin(); itr != m_activeConnections.end(); itr++) {

        m_ioGroup.triggerWrite(*itr);
    }

}