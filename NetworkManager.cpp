
#include "NetworkManager.h"

bool NetworkManager::connectToNode(const char* target, const char* port) {

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
    iResult = getaddrinfo(target, port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        return false;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            return false;
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

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n %ld\n", WSAGetLastError());
        return false;
    }
	
    SOCKADDR_IN client_info = {0};
    int addrsize = sizeof(client_info);

    int client_info_size = sizeof(client_info);
    getpeername(ConnectSocket, (sockaddr*)&client_info, &client_info_size);
    char *ip = inet_ntoa(client_info.sin_addr);
	
    std::cout << "NetworkManager: Client connected: " << std::string(ip) << std::endl;

    std::string name = std::string(ip);

	m_connections.insert({name, ConnectSocket});
	m_activeConnections.push_back(name);

	return true;
}


bool NetworkManager::disconnect(std::string nodeName) {
	
	SOCKET ConnectSocket = m_connections.at(nodeName);
	
	int iResult;
	// shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
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

	SOCKET ConnectSocket = m_connections.at(nodeName);
	
    // Send an initial buffer
	int iResult;
	
	size_t size = buff.getSize();
	
	send( ConnectSocket, (char*)&size, sizeof(size_t), 0);
	
    iResult = send( ConnectSocket, buff.getBase(), buff.getSize(), 0 );
	
	if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        return false;
    }

    printf("Bytes Sent: %ld\n", iResult);
	
	return true;
}

bool NetworkManager::read(std::string nodeName, Buffer& buff) {

    auto itr = m_connections.find(nodeName);

    if(itr == m_connections.end()) {

        std::cout << "NetManager: read: unknown node: " << nodeName << std::endl;

        return false;
    }

	int iResult;
	
	SOCKET clientSocket = m_connections.at(nodeName);
	
	size_t bytesReceived = 0;
	size_t size = 0;

    unsigned long readableBytes = 0;
	
    ioctlsocket(clientSocket, FIONREAD, &readableBytes);

    if(readableBytes == 0) {

        return false;
    }

	recv(clientSocket, (char*)&size, sizeof(size_t), 0);

	for(int i = 0; i < size; i++) {

		char junk = 'f';
		buff.write(&junk, sizeof(char));
	}
	  
    // Receive until the peer shuts down the connection
  do {

        bytesReceived += recv(clientSocket, buff.getBase() + bytesReceived, MAX_BLOCK_SIZE, 0);
		
    } while (bytesReceived < size);
	
	return true;
}

bool NetworkManager::createServer(const char* port) {

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
	
    SOCKADDR_IN client_info = {0};
    int addrsize = sizeof(client_info);

    // or get it from the socket itself at any time
    int client_info_size = sizeof(client_info);
    getpeername(m_listenSocket, (sockaddr*)&client_info, &client_info_size);

    char *ip = inet_ntoa(client_info.sin_addr);

    m_name = std::string(ip);

    std::cout << "NetManager: Server created -> IP: " << m_name << std::endl;

    m_serverCreated = true;

	return true;

}
	
bool NetworkManager::acceptConnection() {
	
    while(m_listening) {
        
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
                return 1;
            }

           // std::cout << "Client listen complete " << std::endl;

            // Accept a client socket
            ClientSocket = accept(m_listenSocket, (struct sockaddr*)&client_info, &addrsize);
            if (ClientSocket == INVALID_SOCKET) {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(m_listenSocket);
                WSACleanup();
                return 1;
            }
            
            char *ip = inet_ntoa(client_info.sin_addr);

            //std::cout << "NetworkManager: Client connected to server: " << std::string(ip) << std::endl;

            std::string name = std::string(ip);

            m_activeConnections.push_back(name);
            m_connections.insert({name, ClientSocket});
            
        }
    }

	return true;
}

void NetworkManager::execute() {

	for(int i = 0; i < m_activeConnections.size(); i++) {

		Buffer buff;
		std::vector<Resource> resources;
		
		read(m_activeConnections[i], buff);

		m_decoder.run(buff, resources);
		
        for(int i = 0; i < resources.size(); i++) {

            std::cout << "NetManager: resource received over network: " << resources[i].type << std::endl;
        }

		putResources(resources);
	}

    std::vector<Resource> res;

    getResources(5, res);

    for(int i = 0; i < res.size(); i++) {

        std::cout << "NetManager: writing resource over network: " << res[i].type << std::endl;
    }


    Buffer buff;
    
    //std::string name = std::string(res[i].target);

    //strcpy(res[i].target, m_name.c_str());

    //std::vector<Resource> temp;

    //for(int i = 0; i < res.size(); i++) {

            //temp.push_back(res[i]);


            // std::vector<Resource> test;

            // m_decoder.run(buff, test);

            // for(int i = 0; i < test.size(); i++) {

            //     std::cout << "NetManager: decoding test: " << test[i].type << std::endl;
            // }


    //}

    m_encoder.run(buff, res);

    if(res.size() > 0) {

        for(int i = 0; i < m_activeConnections.size(); i++) {

            write(m_activeConnections[i], buff);

        }
    }

}