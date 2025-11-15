#ifndef WSANETWORKINGSERVER_H
#define WSANETWORKINGSERVER_H

#include "wsanet/wsa_endpoint.h"
#include "wsanet/wsa_handler.h"

namespace WsaNetworking
{
	class WsaServer : private WsaEndpoint
	{
	public:
		WsaServer(uint16_t port = 55555, const char* ipAddress = "127.0.0.1");

		~WsaServer();

		ConnectionState start();

		void requestStop();
		bool isClientLimitReached();

		template<ConcreteHandler H>
		void acceptNewClients(H* = nullptr);

		template<ConcreteHandler H>
		void sendClientMessage(const char*, uint32_t, size_t, H* = nullptr);

		template<ConcreteHandler H>
		void broadcastClientMessage(const char*, uint32_t, H* = nullptr);

		void sendDummyMessage(size_t); 
		void broadcastDummyMessage(); 
		
	private:
		ConnectionState initializeServer();

		template<ConcreteHandler H>
		void openClientRecvThread(size_t, H* = nullptr);

		void closeClientRecvThread(size_t);

		void stopClient(size_t);
		void stopServer();

		int m_serverPort;
		const char* m_serverIP;
		SOCKET m_serverSocket;
		std::jthread m_acceptClientThread;

		static constexpr size_t m_maxClientCount = 10; // #todo: make configurable; either use std::vector or allocate on the heap
		SOCKET m_clientSockets[m_maxClientCount];
		std::atomic<size_t > m_connectedClientCount;
		std::atomic<size_t > m_maxClientIdx;
		std::atomic<size_t > m_nextClientIdx;
		bool m_isStopRequested;

		std::jthread m_clientConnectionThreads[m_maxClientCount];
		std::mutex m_clientConnectionMutexes[m_maxClientCount];
	};

	template<ConcreteHandler H>
	void WsaServer::sendClientMessage(const char* message, uint32_t messageSize, size_t clientConnection, H* pHandler)
	{
		if (m_clientSockets[clientConnection] == INVALID_SOCKET)
		{
			return;
		}

		if (sendMessageFrame(&m_clientSockets[clientConnection],
			&m_clientConnectionMutexes[clientConnection],
			message,
			messageSize) == ConnectionState::WSACS_OK)
		{
			onMessageSent<H>(message, pHandler);
		}
		else
		{
			stopClient(clientConnection);
		}
	}

	template<ConcreteHandler H>
	void WsaServer::broadcastClientMessage(const char* message, uint32_t messageSize, H* pHandler)
	{
		const size_t activeClients = m_maxClientIdx.load(std::memory_order_acquire);

		for (size_t i = 0; i <= activeClients; i++)
		{
			sendClientMessage<H>(message, messageSize, i, pHandler);
		}
	}

	template<ConcreteHandler H>
	void WsaServer::openClientRecvThread(size_t clientConnectionIdx, H* pHandler)
	{
		m_clientConnectionThreads[clientConnectionIdx] = std::jthread([this, clientConnectionIdx, pHandler](std::stop_token st)
			{
				while (!st.stop_requested())
				{
					WsaMessageFrame inFrame = getMessageFrame(&m_clientSockets[clientConnectionIdx]);

					if (inFrame.state != ConnectionState::WSACS_OK)
					{
						stopClient(clientConnectionIdx);
						break;
					}

					{
						onMessageReceived<H>(inFrame.buffer, pHandler);
					}
				}
			});
	}

	template<ConcreteHandler H>
	void WsaServer::acceptNewClients(H* pHandler)
	{
		m_acceptClientThread = std::jthread([this, pHandler](std::stop_token st)
			{
				while (!st.stop_requested())
				{
					SOCKET acceptedSocket = accept(m_serverSocket, nullptr, nullptr);
					if (acceptedSocket == INVALID_SOCKET)
					{
						std::cout << "accept failed:" << WSAGetLastError() << std::endl;
						return;
					}

					if (!isClientLimitReached())
					{
						const size_t maxClientIdx = m_maxClientIdx.load(std::memory_order_acquire);
						const size_t clientIdx = m_nextClientIdx.load(std::memory_order_acquire);
						m_clientSockets[clientIdx] = acceptedSocket;

						const size_t newMaxIdx = max(maxClientIdx, clientIdx);
						m_connectedClientCount.fetch_add(1, std::memory_order_acq_rel);
						m_maxClientIdx.store(newMaxIdx, std::memory_order_release);
						m_nextClientIdx.store(newMaxIdx + 1, std::memory_order_release);

						std::cout << "client No. " << clientIdx << " accepted!" << std::endl;

						openClientRecvThread<H>(clientIdx, pHandler);
					}
					else
					{
						std::cout << "accept failed: client limit reached" << std::endl;
					}
				}
			});
	}
}

#endif // WSANETWORKINGSERVER_H