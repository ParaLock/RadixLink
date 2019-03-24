
#include "NetworkManager.h"

bool NetworkManager::connectToNode(const char* target, const char* port, bool isSingleton = false) {

    std::string tempTarget = target;
    std::string tempPort   = port;

    m_pendingConnections.push_back(std::make_pair(target, port));

    m_workQueue.addTask(Task(
			"net_connection_thread",
            [tempTarget, tempPort, isSingleton, this]() {

                this->processConnection(tempTarget, tempPort, isSingleton);
            }
	));
    
	return true;
}

void NetworkManager::processConnection(std::string target, std::string port, bool isSingleton = false) {

    Connection connection;
    connection.ip = target;
    connection.port = port;

    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(connection.ip.c_str(), connection.port.c_str(), &hints, &result);

    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        return;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            return;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);


        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    if(ConnectSocket == SOCKET_ERROR) {

        std::string tempTarget = target;
        std::string tempPort   = port;

        m_workQueue.addTask(Task(
                "net_connection_thread",
                [tempTarget, tempPort, this]() {
                    

                    this->processConnection(tempTarget, tempPort);
                }
        ));

        
        return;
    }

    freeaddrinfo(result);
    
    SOCKADDR_IN client_info = {0};
    int addrsize = sizeof(client_info);

    int client_info_size = sizeof(client_info);
    getpeername(ConnectSocket, (sockaddr*)&client_info, &client_info_size);
    char *ip = inet_ntoa(client_info.sin_addr);
    
    std::cout << "NetworkManager: Client connected: " << std::string(ip) << std::endl;

    std::string name = std::string(ip);

    connection.socket = ConnectSocket;

    m_pendingConnections.pop_back();
    m_connections.insert({name, connection});

    m_activeConnections.push_back(name);
}

bool NetworkManager::disconnect(std::string nodeName) {
	
	Connection con = m_connections.at(nodeName);
	
	int iResult;
	// shutdown the connection since no more data will be sent
    iResult = shutdown(con.socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(con.socket);
        return false;
    }
	
	return true;
	
}
	
bool NetworkManager::write(std::string nodeName, Buffer& buff) {

    auto itr = m_connections.find(nodeName);

    if(itr == m_connections.end()) {

        std::cout << "NetManager: write: unknown node: " << nodeName << std::endl;

        return false;
    }

	Connection con = m_connections.at(nodeName);
	
	unsigned int size = buff.getSize();
    size_t bytesSent = 0;
    int iResult = 0;

	send( con.socket, (char*)&size, sizeof(unsigned int), 0);

    do {

        iResult = send(con.socket, buff.getBase() + bytesSent, size - bytesSent, 0);

        if(iResult == SOCKET_ERROR) {

            return false;
        }

        bytesSent += iResult;

    } while(bytesSent < size);
	
    printf("Bytes sent: %ld\n", bytesSent);
	

	return true;
}

bool NetworkManager::read(std::string nodeName, Buffer& buff) {

    auto itr = m_connections.find(nodeName);

    if(itr == m_connections.end()) {

        std::cout << "NetManager: read: unknown node: " << nodeName << std::endl;

        return false;
    }

	Connection con = m_connections.at(nodeName);
	
	size_t bytesReceived = 0;
	unsigned int size = 0;

    u_long readableBytes = 0;
	
    ioctlsocket(con.socket, FIONREAD, &readableBytes);

    if(readableBytes == 0) {

         return true;
    }

    std::cout << "NetManager: Readable bytes on socket: " << readableBytes << std::endl;

	recv(con.socket, (char*)&size, sizeof(unsigned int), 0);

    std::cout << "NetManager: Incoming buffer size: " << size << std::endl;
	
    buff.resize(size);

 //CHECK FOR DISCONNECT: SOCKET_ERROR and WSAGetLastError() returns WSAECONNRESET

    // Receive until the peer shuts down the connection

    int iResult;

   do {
        iResult = recv(con.socket, buff.getBase() + bytesReceived, size - bytesReceived, 0);
		
        if(iResult == SOCKET_ERROR) {

            return false;
        }

        bytesReceived += iResult;

    }  while(bytesReceived < size);
	
    printf("Bytes Received: %ld\n", bytesReceived);
	

	return true;
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

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( m_listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(m_listenSocket);
        WSACleanup();
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
        
        Connection con;

        SOCKADDR_IN client_info = {0};
        int addrsize = sizeof(client_info);

        if(m_serverCreated) {
        
            SOCKET ClientSocket = INVALID_SOCKET;
            
            int iResult;
            
            iResult = listen(m_listenSocket, SOMAXCONN);
            if (iResult == SOCKET_ERROR) {
                printf("listen failed with error: %d\n", WSAGetLastError());
                closesocket(m_listenSocket);
                WSACleanup();
                return;
            }

           // std::cout << "Client listen complete " << std::endl;

            // Accept a client socket
            ClientSocket = accept(m_listenSocket, (struct sockaddr*)&client_info, &addrsize);
            if (ClientSocket == INVALID_SOCKET) {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(m_listenSocket);
                WSACleanup();
                return;
            }
            
            char *ip = inet_ntoa(client_info.sin_addr);

            con.socket = ClientSocket;
            con.ip = ip;


            std::cout << "NetworkManager: Client connected to server: " << std::string(ip) << std::endl;

            std::string name = std::string(ip);

            m_activeConnections.push_back(name);
            m_connections.insert({name, con});
            
        }
    }
}

std::vector<std::string>& NetworkManager::getActiveNodes() {
    return m_activeConnections;
}

void NetworkManager::execute() {

	for(int i = 0; i < m_activeConnections.size(); i++) {

		Buffer buff;
		std::vector<Resource> resources;
		
		if(read(m_activeConnections[i], buff)) {

            m_decoder.run(buff, resources);
            
            for(int i = 0; i < resources.size(); i++) {

                std::cout << "NetManager: resource received over network: " << resources[i].type << std::endl;
            }

            putResources(resources);
        
        } else {

            std::cout << "NetManager: node " << m_activeConnections[i] << " communication failure... " << std::endl;
        }
	}

    std::vector<Resource> res;

    getResources(5, res);

    for(int i = 0; i < res.size(); i++) {

        std::cout << "NetManager: writing resource over network: " << res[i].type << std::endl;
    }

    std::vector<Buffer>      targeted;
    std::vector<std::string> targets;


    Buffer                generalBuff;
    std::vector<Resource> general;

    for(int i = 0; i < res.size(); i++) {

        if(strcmp(res[i].target, "all") == 0) {

            general.push_back(res[i]);

        } else {

            Buffer buff;
            targeted.push_back(buff);

            targets.push_back(res[i].target);
            strcpy(res[i].target, m_name.c_str());

            m_encoder.run(targeted.back(), res[i]);
        }
    }

    m_encoder.run(generalBuff, general);

    if(general.size() > 0) {
        for(int i = 0; i < m_activeConnections.size(); i++) {

            std::cout << "NetManager: sending multi-cast resources..." << std::endl;

            if(!write(m_activeConnections[i], generalBuff)) {

                std::cout << "NetManager: node " << m_activeConnections[i] << " communication failure... " << std::endl;
            }

        }
    }

    for(int i = 0; i < targeted.size(); i++) {
        
        std::cout << "NetManager: sending targeted resource to " << targets[i] << std::endl;

        if(write(targets[i], targeted[i])) {
            
            std::cout << "NetManager: node " << targets[i] << " communication failure... " << std::endl;
        }
    }

    if(isRunning()) {

        Sleep(200);
        m_workQueue.addTask(Task(
			m_worker,
			std::bind(&NetworkManager::execute, this)
		));
    }
}