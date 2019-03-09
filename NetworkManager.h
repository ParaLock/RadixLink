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
#include "Decoder.h"
#include "Encoder.h"
#include "IDispatcher.h"
#include "Resource.h"

const unsigned int MAX_BLOCK_SIZE = 500; 

class NetworkManager : public Manager<NetworkManager> {
private:

	std::map<std::string, SOCKET> m_connections;

	SOCKET  					  m_listenSocket;
	
	std::vector<std::string>      m_activeConnections;
	
	bool                          m_serverCreated;
	bool 						  m_listening;
	std::string m_name;

	Decoder&     m_decoder;
	Encoder&     m_encoder;

	std::thread  m_conListener;
	
public:
	
	NetworkManager(IDispatcher& dispatcher, Decoder& decoder, Encoder& encoder) 
		: Manager(dispatcher, "net_manager"),
		  m_decoder(decoder),
		  m_encoder(encoder)
		  
	{
		m_listenSocket = INVALID_SOCKET;
		m_serverCreated = false;
		m_listening     = true;

		m_conListener = std::thread(&NetworkManager::acceptConnection, this);
	}

	bool connectToNode(const char* target, const char* port);
	bool disconnect(std::string nodeName);
	
	bool write(std::string nodeName, Buffer& buff);
	bool read(std::string nodeName, Buffer& buff);
	
	bool createServer(const char* port);
	bool acceptConnection();
	
	void execute();
	
};