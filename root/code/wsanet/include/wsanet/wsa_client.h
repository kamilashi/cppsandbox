#ifndef WSANETWORKINGCLIENT_H
#define WSANETWORKINGCLIENT_H

#include "wsanet/wsa_endpoint.h"
#include "wsanet/wsa_handler.h"

namespace WsaNetworking
{
	class WsaClient : private WsaEndpoint
	{
	public:
		WsaClient(uint16_t port = 55555, const char* ipAddress = "127.0.0.1") :
			m_serverPort(port),
			m_serverIP(ipAddress),
			m_clientSocket(INVALID_SOCKET),
			m_initializeResult(ConnectionState::WSACS_UNKNOWN)
		{}

		~WsaClient();

		ConnectionState start();

		void requestStop();

		template<ConcreteHandler H>
		void sendServerMessage(const char*, uint32_t, H* = nullptr);

		template<ConcreteHandler H>
		void openServerRecvThread(H* = nullptr);

		void sendDummyMessage();
	private:
		ConnectionState connectToServer();

		void closeServerRecvThread();

		void stopClient();

		int m_serverPort;
		const char* m_serverIP;
		SOCKET m_clientSocket;

		ConnectionState m_initializeResult;

		std::jthread m_serverConnectionThread;
		std::mutex m_mutex;
	};

	template<ConcreteHandler H>
	void WsaClient::sendServerMessage(const char* message, uint32_t messageSize, H* pHandler)
	{
		if (sendMessageFrame(&m_clientSocket,
			&m_mutex,
			message,
			messageSize) == ConnectionState::WSACS_OK)
		{
			onMessageSent(message, pHandler);
		}
		else
		{
			stopClient();
		}
	}

	template<ConcreteHandler H>
	void WsaClient::openServerRecvThread(H* pHandler)
	{
		m_serverConnectionThread = std::jthread([this, pHandler](std::stop_token st)
			{
				while (!st.stop_requested())
				{
					WsaMessageFrame inFrame = getMessageFrame(&m_clientSocket);

					if (inFrame.state != ConnectionState::WSACS_OK)
					{
						stopClient();
						break;
					}

					onMessageReceived<H>(inFrame.buffer, pHandler);
				}
			});
	}
}

#endif // WSANETWORKINGCLIENT_H