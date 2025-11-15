
#include "dataflow/network_factory.h"

namespace Dataflow
{
	namespace NetworkFactory
	{
		std::shared_ptr<WsaNetworking::WsaClient> getCLient()
		{
			static auto client = std::make_shared<WsaNetworking::WsaClient>();
			return client;
		}

		std::shared_ptr<WsaNetworking::WsaServer> getServer()
		{
			static auto server = std::make_shared<WsaNetworking::WsaServer>();
			return server;
		}

	}
}
