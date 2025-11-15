#include "wsanet/wsa_client.h"

namespace WsaNetworking
{
	ConnectionState WsaClient::connectToServer()
	{
		sockaddr_in clientService;
		clientService.sin_family = AF_INET;
		clientService.sin_port = htons(m_serverPort);

		InetPtonA(AF_INET, m_serverIP, &clientService.sin_addr);
		if (connect(m_clientSocket, 
					reinterpret_cast<const sockaddr*>(&clientService), 
					sizeof(clientService)) == SOCKET_ERROR)
		{
			std::cout << "Client connect() failed:" << WSAGetLastError() << std::endl;
			return ConnectionState::WSACS_INITFAIL;
		}

		std::cout << "Client connect(): is OK" << std::endl;
		std::cout << "Client can start sending and receiving data..." << std::endl;

		return ConnectionState::WSACS_OK;
	}

	void WsaClient::closeServerRecvThread()
	{
		m_serverConnectionThread.request_stop();
	}

	ConnectionState WsaClient::start()
	{
		if (m_initializeResult == ConnectionState::WSACS_OK)
		{
			return m_initializeResult;
		}

		m_initializeResult = initializeWSA();

		if (m_initializeResult != ConnectionState::WSACS_OK)
		{
			return m_initializeResult;
		}

		m_initializeResult = createSocket(&m_clientSocket);

		if (m_initializeResult != ConnectionState::WSACS_OK)
		{
			return m_initializeResult;
		}

		m_initializeResult = connectToServer();

		if (m_initializeResult != ConnectionState::WSACS_OK)
		{
			stopClient();
		}

		return m_initializeResult;
	}

	void WsaClient::stopClient()
	{
		closeServerRecvThread();

		if (m_clientSocket != INVALID_SOCKET)
		{
			closesocket(m_clientSocket);
			m_clientSocket = INVALID_SOCKET;
		}

		WSACleanup();
	}

	void WsaClient::requestStop()
	{
		stopClient();
	}

	void WsaClient::sendDummyMessage()
	{
		const char buffer[50] = "Client sent this message!";
		sendServerMessage<DummyHandler>(buffer, strlen(buffer));
	}

	WsaClient::~WsaClient()
	{
		stopClient();
	}
}
