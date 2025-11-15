#ifndef DATAFLOWCLIENTBUSRELAY_H
#define DATAFLOWCLIENTBUSRELAY_H

#include "dataflow/message_bus.h"
#include "dataflow/network_factory.h"
#include "dataflow/serdes.h"
#include <thread>

namespace Dataflow
{
	class NetworkHandler
	{

	};

	class ClientBusRelay
	{
	public:
		ClientBusRelay() : 
			m_spBus(std::make_shared<Bus>()), 
			m_spClient(NetworkFactory::getCLient())
		{
			m_spClient->start();
		}

		~ClientBusRelay() = default;

		std::shared_ptr<Bus> getMessageBus() const
		{
			return m_spBus;
		}

		void relay(const Message& message)
		{
			char serializedMsg[NetworkFactory::sMaxWsaPayloadLength];

			SerDes::serializeMessage(serializedMsg, message);

			m_spClient->sendServerMessage(serializedMsg);

			std::jthread relayThread = std::jthread([&](std::stop_token st)
				{
					while (!st.stop_requested() && !)
					{
					}
				}
			);
		}

	private:
		std::shared_ptr<Bus> m_spBus;
		std::shared_ptr<WsaNetworking::WsaClient> m_spClient; //#wip: generalize!
	};
}

#endif // DATAFLOWCLIENTBUSRELAY_H