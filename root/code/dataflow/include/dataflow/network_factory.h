#ifndef DATAFLOWNETWORKFACTORY_H
#define DATAFLOWNETWORKFACTORY_H

#include "dataflow/network_pipe.h"
#include "dataflow/message_bus.h"
#include <memory>

namespace Dataflow
{
	namespace Ipc
	{
		namespace NetworkFactory
		{
			const std::shared_ptr<Bus>& getMessageBusInstance();

			void initializeClient(NetworkBackend);

			void sendFromClient(const Message&, NetworkBackend);
		}
	}
}

#endif // DATAFLOWNETWORKFACTORY_H