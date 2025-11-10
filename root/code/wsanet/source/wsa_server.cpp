#include "wsanet/wsa_server.h"

namespace WsaNetworking
{
	int WsaServer::startServer()
	{
		WSADATA wsadata;
		int wsaerr;
		WORD wVersionRequested = MAKEWORD(2, 2);
		wsaerr = WSAStartup(wVersionRequested, &wsadata);
		if (wsaerr != 0)
		{
			std::cout << "The winsock dll not found" << std::endl;
			stopServer();
			return -1;
		}

		std::cout << "The winsock dll found" << endl;
		std::cout << "Status: " << wsadata.szSystemStatus << std::endl;

		return 0;
	}

	int WsaServer::createSocket()
	{
		//create socket
		m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (m_serverSocket == INVALID_SOCKET)
		{
			std::cout << "Error at socket():" << WSAGetLastError() << std::endl;
			stopServer();
			return -1;
		}

		std::cout << "socket is OK" << std::endl;

		//bind socket

		sockaddr_in service;
		service.sin_family = AF_INET;
		InetPton(AF_INET, _T(m_serverIP), &service.sin_addr.s_addr);
		service.sin_port = htons(m_serverPort);
		if (bind(m_serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
		{
			std::cout << "bind() failed:" << WSAGetLastError() << std::endl;
			stopServer();
			return -1;
		}

		std::cout << "bind() is OK" << std::endl;

		int clientQueueSize = m_maxClientCount;
		if (listen(m_serverSocket, clientQueueSize) == SOCKET_ERROR)
		{
			std::cout << "listen() : error listening on socket" << WSAGetLastError() << std::endl;
			stopServer();
			return -1;
		}

		std::cout << "Listen() is OK" << std::endl;

		return 0;
	}

	void WsaServer::onMessageReceived(const char* message)
	{
		std::cout << "Message received from client: " << message << std::endl;
	}

	void WsaServer::onMessageSent(const char* message)
	{
		std::cout << "message queued to client: " << message << std::endl;
	}

	ConnectionState WsaServer::sendClientMessage(const char* message, size_t messageLength, size_t clientConnection)
	{
		int justSentByteCount = 0;
		int sentByteCount = 0;

		do
		{
			{
				std::lock_guard<std::mutex> lock(m_clientConnectionMutexes[clientConnection]);

				SOCKET& clientSocket = m_clientSockets[clientConnection];
				justSentByteCount = send(clientSocket, message + sentByteCount, messageLength - sentByteCount, 0);
			}

			if (justSentByteCount <= 0)
			{
				return ConnectionState::WSACS_SENDFAIL;
			}

			sentByteCount += justSentByteCount;
		} while (sentByteCount < messageLength);

		onMessageSent(message);
		
		return ConnectionState::WSACS_OK;
	}

	ConnectionState WsaServer::waitForClientMessage(char* message, size_t messageLength, size_t clientConnection)
	{
		SOCKET& clientSocket = m_clientSockets[clientConnection];

		int recvdByteCount = 0;
		do
		{
			int justRecvdByteCount = recv(clientSocket, message + recvdByteCount, messageLength - recvdByteCount, 0);
			if (justRecvdByteCount <= 0)
			{
				return ConnectionState::WSACS_RECVFAIL;
			}

			recvdByteCount += justRecvdByteCount;
		} while (recvdByteCount < messageLength);

		onMessageReceived(message);

		return ConnectionState::WSACS_OK;
	}

	void WsaServer::openClientRecvThread(size_t clientConnectionIdx)
	{
		m_clientConnectionThreads[clientConnectionIdx] = std::jthread([this, clientConnectionIdx] (std::stop_token st) 
		{
			char messageBuffer[200];

			while (!st.stop_requested())
			{
				ConnectionState state = waitForClientMessage(messageBuffer, 200, clientConnectionIdx);
				
				if (state != ConnectionState::WSACS_OK && shouldStopClient())
				{
					stopClient(clientConnectionIdx);
					break;
				}
			}

		});
	}

	void WsaServer::closeClientRecvThread(size_t clientConnectionIdx)
	{
		m_clientConnectionThreads[clientConnectionIdx].request_stop();
	}

	int WsaServer::acceptNewClient()
	{
		SOCKET acceptedSocket = accept(m_serverSocket, nullptr, nullptr);
		if (acceptedSocket == INVALID_SOCKET)
		{
			std::cout << "accept failed:" << WSAGetLastError() << std::endl;
			return -1;
		}

		const size_t clientIdx = m_connectedClientCount.fetch_add(1, std::memory_order_release);
		m_clientSockets[clientIdx] = acceptedSocket;

		std::cout << "client No. " << clientIdx << " accepted!" << std::endl;

		openClientRecvThread(clientIdx);

		return 0;
	}

	void WsaServer::stopClient(size_t connectionIdx)
	{
		std::lock_guard<std::mutex> lock(m_clientConnectionMutexes[connectionIdx]);

		SOCKET& clientSocket = m_clientSockets[connectionIdx];
		if (clientSocket != INVALID_SOCKET)
		{
			closeClientRecvThread(connectionIdx);

			closesocket(clientSocket);

			clientSocket = INVALID_SOCKET;

			const size_t lastClientIdx = m_connectedClientCount.fetch_sub(1, std::memory_order_relaxed) - 1;
			if (connectionIdx != lastClientIdx)
			{
				m_clientSockets[connectionIdx] = m_clientSockets[lastClientIdx]; // fill up the newly freed space
				m_clientSockets[lastClientIdx] = INVALID_SOCKET;
			}

			std::cout << "stopped client " << connectionIdx << std::endl;
		}
	}

	void WsaServer::stopServer()
	{
		m_acceptClientThread.request_stop();

		for (size_t clientIdx = 0; clientIdx < m_maxClientCount; clientIdx++)
		{
			stopClient(clientIdx); // should all client connections be stopped if the server is down?
		} 

		if (m_serverSocket != INVALID_SOCKET)
		{
			closesocket(m_serverSocket);
			m_serverSocket = INVALID_SOCKET;
		}

		WSACleanup();
	}

	bool WsaServer::isClientLimitReached()
	{
		return m_connectedClientCount.load(std::memory_order_acquire) >= m_maxClientCount;
	}

	void WsaServer::requestStop()
	{
		stopServer();
	}

	void WsaServer::start()
	{
		int ok = startServer();

		if (ok < 0)
		{
			return;
		}
		  
		ok = createSocket();

		if (ok < 0)
		{
			return;
		}

		m_acceptClientThread = std::jthread([this](std::stop_token st)
		{
			while (!st.stop_requested() && !isClientLimitReached())
			{
				acceptNewClient();
			}
		});
	}

	void WsaServer::sendDummyMessage(size_t connectionIdx)
	{
		const char buffer[200] = "broadcasting this!";
		ConnectionState state = sendClientMessage(buffer, 200, connectionIdx);

		if (state != ConnectionState::WSACS_OK && shouldStopClient())
		{
			stopClient(connectionIdx);
		}
	}

	void WsaServer::broadcastDummyMessage() // #wip
	{
		const size_t activeClients = m_connectedClientCount.load(std::memory_order_acquire);

		for (size_t i = 0; i < activeClients; i++)
		{
			sendDummyMessage(i);
		}
	}

	WsaServer::~WsaServer()
	{
		stopServer();
	}
}