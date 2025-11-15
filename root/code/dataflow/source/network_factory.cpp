
#include "dataflow/network_factory.h"

namespace Dataflow
{
	namespace NetworkFactory
	{
		std::shared_ptr<WsaNetworking::WsaClient> getCLient()
		{
			static std::shared_ptr<WsaNetworking::WsaClient> client;
			return client;
		}

		std::shared_ptr<WsaNetworking::WsaServer> getServer()
		{
			static std::shared_ptr<WsaNetworking::WsaServer> server;
			return server;
		}
	}
}
