#include "wsanet/wsa_client.h"

namespace WsaNetworking
{
	void WsaClient::start()
	{
		int ok = startClient();

		if (ok != 0)
		{
			return;
		}

		ok = connectToServer();

		if (ok != 0)
		{
			return;
		}

		openServerRecvThread();
	}

	int WsaClient::startClient()
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

		std::cout << "The winsock dll found" << std::endl;
		std::cout << "Status: " << wsadata.szSystemStatus << std::endl;

		return 0;
	}

	int WsaClient::connectToServer()
	{
		m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (m_clientSocket == INVALID_SOCKET)
		{
			std::cout << "Error at socket():" << WSAGetLastError() << std::endl;
			stopClient();
			return -1;
		}

		std::cout << "socket is OK" << std::endl;

		sockaddr_in clientService;
		clientService.sin_family = AF_INET;
		clientService.sin_port = htons(m_serverPort);

		InetPtonA(AF_INET, m_serverIP, &clientService.sin_addr);
		if (connect(m_clientSocket, 
					reinterpret_cast<const sockaddr*>(&clientService), 
					sizeof(clientService)) == SOCKET_ERROR)
		{
			std::cout << "client connect() failed:" << WSAGetLastError() << std::endl;
			stopClient();
			return -1;
		}

		std::cout << "client connect(): is OK" << std::endl;
		std::cout << "Client can start sending and receiving data..." << std::endl;

		return 0;
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
