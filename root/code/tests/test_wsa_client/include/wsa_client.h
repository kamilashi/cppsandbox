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

		void start()
		{
			startClient();
			connectToServer();
		}

	private:
		int startClient()
		{
			WSADATA wsadata;
			int wsaerr;
			WORD wVersionRequested = MAKEWORD(2, 2);
			wsaerr = WSAStartup(wVersionRequested, &wsadata);
			if (wsaerr != 0)
			{
				std::cout << "The winsock dll not found" << std::endl;
				return -1;
			}
			else 
			{
				std::cout << "The winsock dll found" << std::endl;
				std::cout << "Status: " << wsadata.szSystemStatus << std::endl;
			}

			return 0;
		}

		int connectToServer()
		{
			m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (m_clientSocket == INVALID_SOCKET)
			{
				std::cout << "Error at socket():" << WSAGetLastError() << std::endl;
				WSACleanup();
				return -1;
			}
			else 
			{
				std::cout << "socket is OK" << std::endl;
			}

			sockaddr_in clientService;
			clientService.sin_family = AF_INET;
			clientService.sin_port = htons(m_serverPort);

			InetPtonA(AF_INET, m_serverIP, &clientService.sin_addr);
			if (connect(m_clientSocket, 
						reinterpret_cast<const sockaddr*>(&clientService), 
						sizeof(clientService)) == SOCKET_ERROR)
			{
				std::cout << "client connect() failed:" << WSAGetLastError() << std::endl;
				WSACleanup();
				return -1;
			}
			else
			{
				std::cout << "client connect(): is OK" << std::endl;
				std::cout << "Client can start sending and receiving data..." << std::endl;
			}

			//step 4 send user data to server
			char buffer[200];
			std::cout << "Enter the message to send to server: ";
			cin.getline(buffer, 200);
			int byteCount = send(m_clientSocket, buffer, 200, 0);
			if (byteCount > 0) 
			{
				std::cout << "Message Sent: " << buffer << std::endl;
			}
			else 
			{
				std::cout << "Error sending message" << std::endl;
				WSACleanup();
				return -1;
			}

			//step 5 accept confirmation close the socket and clean resources
			byteCount = recv(m_clientSocket, buffer, 200, 0);
			if (byteCount > 0) 
			{
				std::cout << "Message from server: " << buffer << std::endl;
			}
			else 
			{
				std::cout << "Error receiving message" << std::endl;
				WSACleanup();
				return -1;
			}


			system("pause");
			WSACleanup();
			std::cout << "Closed Socket" << std::endl;
			return 0;
		}

		int m_serverPort;
		const char* m_serverIP;
		SOCKET m_clientSocket;
	};
}

#endif // WSANETWORKINGCLIENT_H