#ifndef WSANETWORKINGSERVER_H
#define WSANETWORKINGSERVER_H

#include <winsock2.h>   
#include <ws2tcpip.h>   
#include <windows.h>  
#include <iostream>

#include <thread>
#include <mutex>

#include "stdafx.h"
#include "wsanet/wsa_helpers.h"

namespace WsaNetworking
{
	class WsaServer
	{
	public:
		WsaServer(uint16_t port = 55555, const char* ipAddress = "127.0.0.1") :
			m_serverPort(port),
			m_serverIP(ipAddress),
			m_isStopRequested(false),
			m_serverSocket(INVALID_SOCKET),
			m_clientSockets{ INVALID_SOCKET },
			m_connectedClientCount(0)
		{}

		~WsaServer();

		void start();
		void requestStop();
		bool isClientLimitReached();

		// #wip: exact API will differ
		void sendDummyMessage(size_t); 
		void broadcastDummyMessage(); 
		
	private:
		int startServer();
		int createSocket();
		int acceptNewClient();

		void openClientRecvThread(size_t);
		void closeClientRecvThread(size_t);
		ConnectionState waitForClientMessage(char*, size_t, size_t);
		ConnectionState sendClientMessage(const char*, size_t, size_t);
		void onMessageReceived(const char*);
		void onMessageSent(const char*);


		void stopClient(size_t);
		void stopServer();

		int m_serverPort;
		const char* m_serverIP;
		bool m_isStopRequested;
		SOCKET m_serverSocket;
		std::jthread m_acceptClientThread;

		static constexpr size_t m_maxClientCount = 10; // #todo: make configurable; either use std::vector or allocate on the heap
		SOCKET m_clientSockets[m_maxClientCount];
		std::atomic<size_t > m_connectedClientCount;

		std::jthread m_clientConnectionThreads[m_maxClientCount];
		std::mutex m_clientConnectionMutexes[m_maxClientCount];
	};
}

#endif // WSANETWORKINGSERVER_H