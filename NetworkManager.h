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
#include <set>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"

#include "Manager.h"
#include "Buffer.h"
#include "Decoder.h"
#include "Encoder.h"
#include "IDispatcher.h"
#include "Resource.h"
#include "StreamGroup.h"
#include "TaskExecutor.h"

const unsigned int MAX_BLOCK_SIZE = 512; 
const unsigned int MAX_IO_THREADS = 4;

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

	std::set<std::string>						      m_activeConnections;
	

	SOCKET  					  			 		  m_listenSocket;

	bool                          			 		  m_serverCreated;
	bool 						 			 		  m_listening;
	bool 											  m_connecting;
	std::string 							 		  m_name;

	std::string										  m_debugMode;

	bool 									          m_monitorConnected;

	NetIO::StreamGroup                                m_ioGroup;

	Decoder&     						     	 	  m_decoder;
	Encoder&     						     		  m_encoder;

public:
	
	NetworkManager(IDispatcher& dispatcher, TaskExecutor& executor, StateRegistry& reg, Decoder& decoder, Encoder& encoder) 
		: Manager(dispatcher, executor, reg, "net_manager", false),
		  m_decoder(decoder),
		  m_encoder(encoder),
		  m_ioGroup(executor)
		  
	{
		m_debugMode = "";

		m_listenSocket = INVALID_SOCKET;
		m_serverCreated = false;
		m_listening     = true;
		m_connecting    = true;
		m_monitorConnected = false;
		
		m_stateReg.addState<std::string>("writing_too", "none");
		m_stateReg.addState<std::string>("reading_from", "none");
		
		m_workExecutor.createQueue("net_monitoring", 1);
		m_workExecutor.createQueue("net_connection", 1);
		m_workExecutor.createQueue("net_listen_thread", 1);
		
		executor.addTask(Task(
			"net_listen_thread",
			std::bind(&NetworkManager::acceptConnection, this)
		));

		//SecureZeroMemory(&testOverlapped, sizeof(testOverlapped));

	}

	~NetworkManager() {


	}

	std::string testSocket;

	void testRead();

	void testWrite();
	std::vector<std::string> getActiveNodes();

	bool connectToNode(const char* target, const char* port);
	bool disconnect(std::string nodeName);
	
	void processConnection(std::string target, std::string port);

	void monitoringLoop();

	bool write(std::string nodeName, Buffer& buff, bool updateState);
	bool read(std::string nodeName, Buffer& buff, bool updateState);
	
	bool createServer(std::string ip,const char* port);
	void acceptConnection();
	
	void customShutdown();

	void execute();

	void setDebugMode(std::string mode) {
		m_debugMode = mode;
	}

};