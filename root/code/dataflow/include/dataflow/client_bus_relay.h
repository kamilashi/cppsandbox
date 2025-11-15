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
	public:
		NetworkHandler() : 
			m_isLastMessageFullyQueued (false),
			m_spBus(std::make_shared<Bus>())
		{ }

		void onMessageQueued(const char*)
		{
			m_isLastMessageFullyQueued = true;
		}

		void onMessageReceived(const char* msg)
		{
			Message desMessage;
			SerDes::deserializeMessage(&desMessage, msg);

			m_spBus->publish(desMessage);
		}

		bool consumeMessageQueuedFlag()
		{
			bool flag = m_isLastMessageFullyQueued;
			m_isLastMessageFullyQueued = false;
			return flag;
		}

		std::shared_ptr<Bus>& getMessageBus()
		{
			return m_spBus;
		}
	private:
		bool m_isLastMessageFullyQueued;
		std::shared_ptr<Bus> m_spBus;
	};

	class ClientBusRelay
	{
	public:
		ClientBusRelay() : 
			m_spClient(NetworkFactory::getClient())
		{
			m_spClient->start();
			m_spClient->openServerRecvThread<NetworkHandler>(&m_recieveHandler);
		}

		~ClientBusRelay() = default;

		std::shared_ptr<Bus>& getMessageBus()
		{
			return m_recieveHandler.getMessageBus();
		}

		void relay(const Message& message)
		{
			std::jthread relayThread = std::jthread([this, &message ](std::stop_token st)
				{	
					char serializedMsg[NetworkFactory::sMaxWsaPayloadLength];
					uint32_t messageSize;

					SerDes::serializeMessage(serializedMsg, message, &messageSize);

					NetworkHandler handler;
					m_spClient->sendServerMessage<NetworkHandler>(serializedMsg, messageSize, &handler);

					while (!st.stop_requested() && !handler.consumeMessageQueuedFlag())
					{ };
				}
			);
		}

	private:
		//#wip: generalize and pack into a NetworkAdapter!
		std::shared_ptr<WsaNetworking::WsaClient> m_spClient; 
		NetworkHandler m_recieveHandler;
	};
}

#endif // DATAFLOWCLIENTBUSRELAY_H