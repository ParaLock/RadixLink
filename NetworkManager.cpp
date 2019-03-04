
#include "NetworkManager.h"

bool NetworkManager::connectToNode(std::string nodeName, const char* target, const char* port) {

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
    iResult = getaddrinfo(target, DEFAULT_PORT, &hints, &result);
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
        printf("Unable to connect to server!\n");
        return false;
    }
	
	
	
	connections.insert({nodeName, ConnectSocket});
	
	return true;
}


bool NetworkManager::disconnect(std::string nodeName) {
	
	SOCKET ConnectSocket = connections.at(nodeName);
	
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
		
	SOCKET ConnectSocket = connections.at(nodeName);
	
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
	
	int iResult;
	
	SOCKET clientSocket = clients.at(nodeName);
	
	size_t bytesReceived = 0;
	size_t size = 0;
		
	recv(clientSocket, (char*)&size, sizeof(size_t), 0);

	for(int i = 0; i < size; i++) {

		char junk = 'f';
		buff.write(&junk, sizeof(char));
	}
	  
	
    // Receive until the peer shuts down the connection
  do {

        bytesReceived += recv(clientSocket, buff.getBase() + bytesReceived, MAX_BLOCK_SIZE, 0);
		
       // if (iResult > 0) {
			
         //   printf("Bytes received: %d\n", iResult);

        //}
        //else if (iResult == 0) {

         //   printf("Connection closing...\n");
		//}

        //else  {
        //    printf("recv failed with error: %d\n", WSAGetLastError());
        //    return 1;
       // }
		
    } while (bytesReceived < size);
	
	return true;
}

bool NetworkManager::createServer(std::string nodeName, const char* port) {

    struct addrinfo *result = NULL;
    struct addrinfo hints;
	
	int iResult;
	
	SOCKET ListenSocket = INVALID_SOCKET;
	
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
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);
	
	listeners.insert({nodeName, ListenSocket});
	
    m_thisNode = nodeName;

	return true;

}
	
bool NetworkManager::acceptConnection(std::string clientName, std::string serverName) {
	
	
    SOCKET ClientSocket = INVALID_SOCKET;
	SOCKET ListenSocket = listeners.at(serverName);
	
    int iResult;
	
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

	std::cout << "Client listen complete " << std::endl;

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);
	
	activeClients.push_back(clientName);
	
	clients.insert({clientName, ClientSocket});
	
	return true;
}

void NetworkManager::execute() {

	for(int i = 0; i < activeClients.size(); i++) {
	
		Buffer buff;
		std::vector<Resource> resources;
		
		read(activeClients[i], buff);
		
		m_decoder.run(buff, resources);
		
		putResources(resources);
	}

    std::vector<Resource> res = getResources(5);

    for(int i = 0; i < res.size(); i++) {

        Buffer buff;
        
        std::string name = std::string(res[i].sendername);

        strcpy(res[i].senderName, m_thisNode.c_str());

        m_encoder.run(buff, {res[i]});

        write(name, buff);

    }
}