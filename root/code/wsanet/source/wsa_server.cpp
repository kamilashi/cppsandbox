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

	void WsaServer::closeClientRecvThread(size_t clientConnectionIdx)
	{
		m_clientConnectionThreads[clientConnectionIdx].request_stop();
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

			m_connectedClientCount.fetch_sub(1, std::memory_order_acq_rel);
			m_nextClientIdx.store(connectionIdx, std::memory_order_release); 

			std::cout << "stopped client " << connectionIdx << std::endl;
		}
	}

	bool WsaServer::isClientLimitReached()
	{
		return m_connectedClientCount.load(std::memory_order_acquire) >= m_maxClientCount;
	}

	ConnectionState WsaServer::start()
	{
		ConnectionState state = initializeWSA();

		if (state != ConnectionState::WSACS_OK)
		{
			return state;
		}
		
		state = createSocket(&m_serverSocket);

		if (state != ConnectionState::WSACS_OK)
		{
			return state;
		}
		  
		state = initializeServer();

		if (state != ConnectionState::WSACS_OK)
		{
			stopServer();
			return state;
		}

		m_connectedClientCount.store(0, std::memory_order_release);
		m_nextClientIdx.store(0, std::memory_order_release);
		m_maxClientIdx.store(0, std::memory_order_release);

		return state;
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
		const char buffer[50] = "broadcasting this!";
		sendClientMessage<DummyHandler>(buffer, 50, connectionIdx);
	}

	void WsaServer::broadcastDummyMessage() // #wip
	{
		const size_t activeClients = m_maxClientIdx.load(std::memory_order_acquire);

		for (size_t i = 0; i <= activeClients; i++)
		{
			if (m_clientSockets[i] != INVALID_SOCKET)
			{
				sendDummyMessage(i);
			}
		}
	}

	WsaServer::WsaServer(uint16_t port, const char* ipAddress) :
		m_serverPort(port),
		m_serverIP(ipAddress),
		m_serverSocket(INVALID_SOCKET),
		m_connectedClientCount(0),
		m_isStopRequested(false)
	{
		for (auto& socket : m_clientSockets) 
		{
			socket = INVALID_SOCKET;
		}
	}

	WsaServer::~WsaServer()
	{
		stopServer();
	}
}