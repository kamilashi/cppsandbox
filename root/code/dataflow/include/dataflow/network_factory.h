#ifndef DATAFLOWNETWORKFACTORY_H
#define DATAFLOWNETWORKFACTORY_H

#include "wsanet/wsa_client.h"
#include "wsanet/wsa_server.h"
#include <memory>

namespace Dataflow
{
	namespace NetworkFactory
	{
		std::shared_ptr<WsaNetworking::WsaClient> getCLient();

		std::shared_ptr<WsaNetworking::WsaServer> getServer();
	}
}

#endif // DATAFLOWNETWORKFACTORY_H