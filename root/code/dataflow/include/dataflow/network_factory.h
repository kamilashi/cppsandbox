#ifndef DATAFLOWNETWORKFACTORY_H
#define DATAFLOWNETWORKFACTORY_H

#include "wsanet/wsa_client.h"
#include "wsanet/wsa_server.h"
#include "wsanet/wsa_handler.h"
#include <memory>

namespace Dataflow
{
	namespace NetworkFactory
	{
		static constexpr uint32_t sMaxWsaPayloadLength = 1024;

		std::shared_ptr<WsaNetworking::WsaClient> getCLient();

		std::shared_ptr<WsaNetworking::WsaServer> getServer();
	}
}

#endif // DATAFLOWNETWORKFACTORY_H