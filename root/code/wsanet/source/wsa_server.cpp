#include "wsanet/wsa_server.h"

namespace WsaNetworking
{
	ConnectionState WsaServer::initializeServer()
	{
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_port = htons(m_serverPort);

		InetPtonA(AF_INET, m_serverIP, &service.sin_addr);
		if (bind(m_serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
		{
			std::cout << "Bind() failed:" << WSAGetLastError() << std::endl;
			return ConnectionState::WSACS_INITFAIL;
		}

		std::cout << "Bind() is OK" << std::endl;

		int clientQueueSize = m_maxClientCount;
		if (listen(m_serverSocket, clientQueueSize) == SOCKET_ERROR)
		{
			std::cout << "Listen() : error listening on socket" << WSAGetLastError() << std::endl;
			return ConnectionState::WSACS_INITFAIL;
		}

		std::cout << "Listen() is OK" << std::endl;

		return ConnectionState::WSACS_OK;
	}

	void WsaServer::onMessageReceived(const char* message)
	{
		std::cout << "Message received from client: " << message << std::endl;
	}

	void WsaServer::onMessageSent(const char* message)
	{
		std::cout << "message queued to client: " << message << std::endl;
	}

	void WsaServer::sendClientMessage(const char* message, size_t clientConnection)
	{
		if (sendMessageFrame(&m_clientSockets[clientConnection],
			&m_clientConnectionMutexes[clientConnection],
			message) == ConnectionState::WSACS_OK)
		{
			onMessageSent(message);
		}
		else if (isFatalError())
		{
			stopClient(clientConnection);
		}
	}

	ConnectionState WsaServer::waitForClientMessage(size_t clientConnection)
	{
		WsaMessageFrame frame = getMessageFrame(&m_clientSockets[clientConnection]);

		if (frame.state == ConnectionState::WSACS_OK)
		{
			onMessageReceived(frame.buffer);
		}

		return frame.state;
	}

	void WsaServer::openClientRecvThread(size_t clientConnectionIdx)
	{
		m_clientConnectionThreads[clientConnectionIdx] = std::jthread([this, clientConnectionIdx] (std::stop_token st) 
		{
			while (!st.stop_requested())
			{
				ConnectionState state = waitForClientMessage(clientConnectionIdx);
				
				if (state != ConnectionState::WSACS_OK && isFatalError())
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

	bool WsaServer::isClientLimitReached()
	{
		return m_connectedClientCount.load(std::memory_order_acquire) >= m_maxClientCount;
	}

	void WsaServer::start()
	{
		ConnectionState state = initializeWSA();

		if (state != ConnectionState::WSACS_OK)
		{
			return;
		}
		
		state = createSocket(&m_serverSocket);

		if (state != ConnectionState::WSACS_OK)
		{
			return;
		}
		  
		state = initializeServer();

		if (state != ConnectionState::WSACS_OK)
		{
			stopServer();
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

	void WsaServer::requestStop()
	{
		stopServer();
	}

	void WsaServer::sendDummyMessage(size_t connectionIdx)
	{
		const char buffer[200] = "broadcasting this!";
		sendClientMessage(buffer, connectionIdx);
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