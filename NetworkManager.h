#pragma once
#define WIN32_LEAN_AND_MEAN

#undef UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <map>
#include <string>
#include <iostream>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#include "Manager.h"
#include "Buffer.h"

const unsigned int MAX_BLOCK_SIZE = 500; 

class NetworkManager : public Manager<NetworkManager> {
private:
	std::map<std::string, SOCKET> m_connections;
	std::map<std::string, SOCKET> m_clients;
	std::map<std::string, SOCKET> m_listeners;
	
	std::vector<std::string>      m_activeClients;
	
	Decoder&     m_decoder;
	Encoder&     m_encoder;
	
public:
	
	NetworkManager(IDispatcher& dispatcher, Decoder& decoder, Encoder& encoder) 
		: Manager(dispatcher),
		  m_decoder(decoder),
		  m_encoder(encoder)
		  
	{
		
	}
	
	bool connectToNode(std::string nodeName, const char* target, const char* port);
	bool disconnect(std::string nodeName);
	
	bool write(std::string nodeName, Buffer& buff);
	bool read(std::string nodeName, Buffer& buff);
	
	bool createServer(std::string nodeName, const char* port);
	bool acceptConnection(std::string nodeName, std::string serverName);
	
	void execute();
	
};