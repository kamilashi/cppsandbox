#include "wsanet/wsa_server.h"
#include "dataflow/message.h"
#include "dataflow/serdes.h"

#include <iostream>
#include <mutex>

namespace Dataflow
{
	class ServerHandler
	{
	public:
		ServerHandler() : m_pServer(nullptr) {};
		ServerHandler(WsaNetworking::WsaServer* pServer) : m_pServer(pServer)
		{}
		~ServerHandler() {};

		void onMessageReceived(const char* message)
		{
			std::lock_guard<std::mutex> lock(m_handlerMutex);

			if (m_pServer != nullptr)
			{
				m_pServer->broadcastClientMessage<ServerHandler>(message, 1024);
			}
		}

		void onMessageQueued(const char* message)
		{
			Message desMessage;
			SerDes::deserializeMessageWSA(&desMessage, message);
			//std::cout << "message broadcasted to clients: " << desMessage << std::endl;
		}

	private:
		std::mutex m_handlerMutex;
		WsaNetworking::WsaServer* m_pServer;
	};

	int testSever()
	{
		WsaNetworking::WsaServer server;

		ServerHandler handler(&server);

		server.start();
		server.acceptNewClients<ServerHandler>(&handler);

		while (getchar() != '\n')
		{
		}

		return 0;
	}

	int runTest()
	{
		return testSever();
	}
}


int main(int argc, char* argv[])
{
	return Dataflow::runTest();
}