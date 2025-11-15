#ifndef DATAFLOWCLIENTBUSRELAY_H
#define DATAFLOWCLIENTBUSRELAY_H

#include "dataflow/message_bus.h"
#include "dataflow/network_factory.h"
#include "dataflow/serdes.h"
#include <thread>

namespace Dataflow
{
	struct NetworkHandler
	{
		bool isLastMessageFullyQueued;

		NetworkHandler() : 
			isLastMessageFullyQueued (false)
		{ }

		void onMessageQueued(const char*)
		{
			isLastMessageFullyQueued = true;
		}

		void onMessageReceived(const char* msg)
		{
			std::cout << "Message from server received! \n" << msg << std::endl;
		}

		bool consumeMessageQueuedFlag()
		{
			bool flag = isLastMessageFullyQueued;
			isLastMessageFullyQueued = false;
			return flag;
		}
	};

	class ClientBusRelay
	{
	public:
		ClientBusRelay() : 
			m_spBus(std::make_shared<Bus>()), 
			m_spClient(NetworkFactory::getCLient())
		{
			m_spClient->start();
			m_spClient->openServerRecvThread<NetworkHandler>(&m_recieveHandler);
		}

		~ClientBusRelay() = default;

		std::shared_ptr<Bus> getMessageBus() const
		{
			return m_spBus;
		}

		void relay(const Message& message)
		{
			char serializedMsg[NetworkFactory::sMaxWsaPayloadLength];
			uint32_t messageSize;

			SerDes::serializeMessage(serializedMsg, message, &messageSize);

			NetworkHandler handler;
			m_spClient->sendServerMessage<NetworkHandler>(serializedMsg, messageSize, &handler);

			std::jthread relayThread = std::jthread([&](std::stop_token st)
				{
					while (!st.stop_requested() && !handler.consumeMessageQueuedFlag())
					{
					};
				}
			);
		}

	private:
		std::shared_ptr<Bus> m_spBus;
		//#wip: generalize and pack into a NetworkAdapter!
		std::shared_ptr<WsaNetworking::WsaClient> m_spClient; 
		NetworkHandler m_recieveHandler;
	};
}

#endif // DATAFLOWCLIENTBUSRELAY_H