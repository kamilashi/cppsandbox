#ifndef WSANETWORKINGCLIENT_H
#define WSANETWORKINGCLIENT_H

#include <winsock2.h>   
#include <ws2tcpip.h>   
#include <windows.h>  
#include <iostream>

#include "stdafx.h"

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

		void start();

	private:
		int startClient();
		int connectToServer();

		int m_serverPort;
		const char* m_serverIP;
		SOCKET m_clientSocket;
	};
}

#endif // WSANETWORKINGCLIENT_H