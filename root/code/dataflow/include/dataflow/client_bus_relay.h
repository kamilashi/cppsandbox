#ifndef DATAFLOWCLIENTBUSRELAY_H
#define DATAFLOWCLIENTBUSRELAY_H

#include "dataflow/message_bus.h"
#include "dataflow/network_factory.h"

namespace Dataflow
{
	class ClientBusRelay
	{
	public:
		ClientBusRelay() : 
			m_spBus(std::make_shared<Bus>()), 
			m_spClient(NetworkFactory::getCLient())
		{}

		~ClientBusRelay() = default;

		std::shared_ptr<Bus> getMessageBus() const
		{
			return m_spBus;
		}

		void relay(const Message& message)
		{

		}

	private:
		std::shared_ptr<Bus> m_spBus;
		std::shared_ptr <WsaNetworking::WsaClient> m_spClient;
	};
}

#endif // DATAFLOWCLIENTBUSRELAY_H