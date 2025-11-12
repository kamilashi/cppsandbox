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

	void WsaClient::onMessageReceived(const char* message)
	{
		std::cout << "Message received from server: " << message << std::endl;
	}

	void WsaClient::onMessageSent(const char* message)
	{
		std::cout << "message queued to server: " << message << std::endl;
	}

	void WsaClient::sendServerMessage(const char* message)
	{
		if (sendMessageFrame(&m_clientSocket, 
						&m_mutex, 
						message) == ConnectionState::WSACS_OK)
		{
			onMessageSent(message);
		}
		else if(isFatalError())
		{
			//stopClient();
		}
	}

	ConnectionState WsaClient::waitForServerMessage()
	{
		WsaMessageFrame inFrame = getMessageFrame(&m_clientSocket);

		if (inFrame.state == ConnectionState::WSACS_OK)
		{
			onMessageReceived(inFrame.buffer);
		}

		return inFrame.state;
	}

	void WsaClient::openServerRecvThread()
	{
		m_serverConnectionThread = std::jthread([this](std::stop_token st)
		{
			while (!st.stop_requested())
			{
				ConnectionState state = waitForServerMessage();

				if (state != ConnectionState::WSACS_OK && isFatalError())
				{
					//stopClient();
					break;
				}
			}
		});
	}

	void WsaClient::closeServerRecvThread()
	{
		m_serverConnectionThread.request_stop();
	}

	void WsaClient::start()
	{
		ConnectionState state = initializeWSA();

		if (state != ConnectionState::WSACS_OK)
		{
			return;
		}

		state = createSocket(&m_clientSocket);

		if (state != ConnectionState::WSACS_OK)
		{
			return;
		}

		state = connectToServer();

		if (state != ConnectionState::WSACS_OK)
		{
			stopClient();
			return;
		}

		openServerRecvThread();
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
		const char buffer[200] = "Client sent this message!";
		sendServerMessage(buffer);
	}

	WsaClient::~WsaClient()
	{
		stopClient();
	}
}
