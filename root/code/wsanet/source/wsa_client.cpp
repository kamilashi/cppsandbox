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

	ConnectionState WsaClient::sendServerMessage(const char* message, size_t messageLength)
	{
		int justSentByteCount = 0;
		int sentByteCount = 0;

		do
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				SOCKET& clientSocket = m_clientSocket;
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

	ConnectionState WsaClient::waitForServerMessage(char* message, size_t messageLength)
	{
		SOCKET& clientSocket = m_clientSocket;

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

	void WsaClient::openServerRecvThread()
	{
		m_serverConnectionThread = std::jthread([this](std::stop_token st)
			{
				char messageBuffer[200];

				while (!st.stop_requested())
				{
					ConnectionState state = waitForServerMessage(messageBuffer, 200);

					if (state != ConnectionState::WSACS_OK && shouldStopClient())
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
		ConnectionState state = sendServerMessage(buffer, 200);

		if (state != ConnectionState::WSACS_OK && shouldStopClient())
		{
			//stopClient();
		}
	}

	WsaClient::~WsaClient()
	{
		stopClient();
	}
}
