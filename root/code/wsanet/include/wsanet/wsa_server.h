#ifndef WSANETWORKINGSERVER_H
#define WSANETWORKINGSERVER_H

#include "wsanet/wsa_endpoint.h"

namespace WsaNetworking
{
	class WsaServer : private WsaEndpoint
	{
	public:
		WsaServer(uint16_t port = 55555, const char* ipAddress = "127.0.0.1") :
			m_serverPort(port),
			m_serverIP(ipAddress),
			m_serverSocket(INVALID_SOCKET),
			m_clientSockets{ INVALID_SOCKET },
			m_connectedClientCount(0),
			m_isStopRequested(false)
		{}

		~WsaServer();

		void start();
		void requestStop();
		bool isClientLimitReached();

		void sendClientMessage(const char*, size_t);

		void sendDummyMessage(size_t); 
		void broadcastDummyMessage(); 
		
	private:
		ConnectionState initializeServer();
		int acceptNewClient();

		void onMessageReceived(const char*);
		void onMessageSent(const char*);

		ConnectionState waitForClientMessage(size_t);
		void openClientRecvThread(size_t);
		void closeClientRecvThread(size_t);

		void stopClient(size_t);
		void stopServer();

		int m_serverPort;
		const char* m_serverIP;
		SOCKET m_serverSocket;
		std::jthread m_acceptClientThread;

		static constexpr size_t m_maxClientCount = 2; // #todo: make configurable; either use std::vector or allocate on the heap
		SOCKET m_clientSockets[m_maxClientCount];
		std::atomic<size_t > m_connectedClientCount;
		std::atomic<size_t > m_maxClientIdx;
		std::atomic<size_t > m_nextClientIdx;
		bool m_isStopRequested;

		std::jthread m_clientConnectionThreads[m_maxClientCount];
		std::mutex m_clientConnectionMutexes[m_maxClientCount];
	};
}

#endif // WSANETWORKINGSERVER_H