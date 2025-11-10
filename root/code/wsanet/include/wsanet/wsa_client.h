#ifndef WSANETWORKINGCLIENT_H
#define WSANETWORKINGCLIENT_H

#include <winsock2.h>   
#include <ws2tcpip.h>   
#include <windows.h>  
#include <iostream>
#include <thread>
#include <mutex>

#include "stdafx.h"
#include "wsa_helpers.h"

namespace WsaNetworking
{
	class WsaClient
	{
	public:
		WsaClient() : 
			m_serverPort(55555),
			m_serverIP("127.0.0.1"),
			m_clientSocket(INVALID_SOCKET)
		{}

		~WsaClient();

		void start();
		void sendDummyMessage();
		void requestStop();

	private:
		int startClient();
		int connectToServer();
		void stopClient();

		void onMessageReceived(const char*);
		void onMessageSent(const char*);
		ConnectionState waitForServerMessage(char*, size_t);
		ConnectionState sendServerMessage(const char*, size_t);
		void openServerRecvThread();
		void closeServerRecvThread();

		int m_serverPort;
		const char* m_serverIP;
		SOCKET m_clientSocket;

		std::jthread m_serverConnectionThread;
		std::mutex m_mutex;
	};
}

#endif // WSANETWORKINGCLIENT_H