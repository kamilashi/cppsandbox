#include "dataflow/client_bus_relay.h"
#include "dataflow/network_factory.h"

namespace Dataflow
{
	ClientBusRelay::ClientBusRelay(NetworkBackend networkBackend) :
		m_neworkPipe(networkBackend)
	{
		m_neworkPipe.initializeClient();
	}

	void ClientBusRelay::relay(const Message& message)
	{
		m_neworkPipe.send(message);
	}

	std::shared_ptr <Bus> ClientBusRelay::getMessageBus() const
	{
		return NetworkFactory::getMessageBusInstance();
	}
}