#include "dataflow/network_pipe.h"
#include "dataflow/network_factory.h"

namespace Dataflow 
{
	namespace Ipc
	{
		NetworkPipe::NetworkPipe(NetworkBackend backend) : 
			m_netBackend(backend), 
			m_netType(NetworkType::NetworkType_Unknown)
		{}

		void NetworkPipe::initializeClient()
		{
			m_netType = NetworkType::NetworkType_Client;
			NetworkFactory::initializeClient(m_netBackend);
		}

		void NetworkPipe::initializeServer()
		{
			m_netType = NetworkType::NetworkType_Server;
		}

		void NetworkPipe::send(const Message& message)
		{
			NetworkFactory::sendFromClient(message, m_netBackend);
		}
	}
}

