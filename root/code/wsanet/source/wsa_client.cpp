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
		ConnectionState state = initializeWSA();

		if (state != ConnectionState::WSACS_OK)
		{
			return state;
		}

		state = createSocket(&m_clientSocket);

		if (state != ConnectionState::WSACS_OK)
		{
			return state;
		}

		state = connectToServer();

		if (state != ConnectionState::WSACS_OK)
		{
			stopClient();
		}

		return state;
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
		sendServerMessage<DummyHandler>(buffer);
	}

	WsaClient::~WsaClient()
	{
		stopClient();
	}
}
