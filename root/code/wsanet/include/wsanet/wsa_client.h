#ifndef WSANETWORKINGCLIENT_H
#define WSANETWORKINGCLIENT_H

#include "wsanet/wsa_endpoint.h"

namespace WsaNetworking
{
	class WsaClient : private WsaEndpoint
	{
	public:
		WsaClient(uint16_t port = 55555, const char* ipAddress = "127.0.0.1") :
			m_serverPort(port),
			m_serverIP(ipAddress),
			m_clientSocket(INVALID_SOCKET)
		{}

		~WsaClient();

		void start();
		void sendDummyMessage();
		void requestStop();

	private:
		ConnectionState connectToServer();

		void onMessageReceived(const char*);
		void onMessageSent(const char*);
		ConnectionState waitForServerMessage(char*, size_t);
		ConnectionState sendServerMessage(const char*, size_t);
		void openServerRecvThread();
		void closeServerRecvThread();

		void stopClient();

		int m_serverPort;
		const char* m_serverIP;
		SOCKET m_clientSocket;

		std::jthread m_serverConnectionThread;
		std::mutex m_mutex;
	};
}

#endif // WSANETWORKINGCLIENT_H