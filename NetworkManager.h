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
#define DEFAULT_PORT "27016"

#include "Manager.h"
#include "Buffer.h"
#include "Decoder.h"
#include "Encoder.h"
#include "IDispatcher.h"
#include "Resource.h"
#include "TaskQueue.h"

const unsigned int MAX_BLOCK_SIZE = 512; 

class NetworkManager : public Manager<NetworkManager> {
private:

	struct Connection {

		uint64_t lastActive;
		SOCKET   socket;
		std::string ip;
		std::string port;

		Connection() {

			socket = INVALID_SOCKET;

		}
	};

	std::map<std::string, Connection> 		 		  m_connections;

	std::vector<std::pair<std::string, std::string>>  m_pendingConnections;
	std::vector<std::string>	             		  m_deadConnections;
	std::vector<std::string>      			 		  m_activeConnections;
	

	SOCKET  					  			 		  m_listenSocket;

	bool                          			 		  m_serverCreated;
	bool 						 			 		  m_listening;
	bool 											  m_connecting;
	std::string 							 		  m_name;

	bool 									          m_monitorConnected;

	Decoder&     						     	 	  m_decoder;
	Encoder&     						     		  m_encoder;

public:
	
	NetworkManager(IDispatcher& dispatcher, TaskQueue& queue, StateRegistry& reg, Decoder& decoder, Encoder& encoder) 
		: Manager(dispatcher, queue, reg, "net_manager", true),
		  m_decoder(decoder),
		  m_encoder(encoder)
		  
	{
		m_listenSocket = INVALID_SOCKET;
		m_serverCreated = false;
		m_listening     = true;
		m_connecting    = true;
		m_monitorConnected = false;
		
		
		m_stateReg.addState<std::string>("writing_too", "none");
		m_stateReg.addState<std::string>("reading_from", "none");
		
		queue.addTask(Task(
			"net_listen_thread",
			std::bind(&NetworkManager::acceptConnection, this)
		));


		queue.addTask(Task(
			"net_monitoring_thread",
			std::bind(&NetworkManager::monitoringLoop, this)
		));
	}

	std::vector<std::string>& getActiveNodes();

	bool dataReady(std::string socket);

	bool connectToNode(const char* target, const char* port, bool isSingleton);
	bool disconnect(std::string nodeName);
	
	void processConnection(std::string target, std::string port, bool isSingletor);

	void monitoringLoop();

	bool write(std::string nodeName, Buffer& buff);
	bool read(std::string nodeName, Buffer& buff);
	
	bool createServer(std::string ip,const char* port);
	void acceptConnection();
	
	void execute();
	
};