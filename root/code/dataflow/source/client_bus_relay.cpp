#include "dataflow/client_bus_relay.h"
#include "dataflow/network_factory.h"

namespace Dataflow
{
	namespace Ipc
	{
		ClientBusRelay::ClientBusRelay(NetworkBackend networkBackend) :
			m_networkPipe(networkBackend)
		{
			m_networkPipe.initializeClient();
		}

		void ClientBusRelay::relay(const Message & message)
		{
			m_networkPipe.send(message);
		}

		std::shared_ptr <Bus> ClientBusRelay::getMessageBus() const
		{
			return NetworkFactory::getMessageBusInstance();
		}
	}
}