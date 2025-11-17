
#include "dataflow/network_factory.h"
#include "dataflow/serdes.h"

#include "wsanet/wsa_client.h"
#include "wsanet/wsa_server.h"
#include "wsanet/wsa_handler.h"


namespace Dataflow
{
	namespace NetworkFactory
	{
		static constexpr uint32_t sMaxWsaPayloadLength = 1024;

		class WsaNetworkHandler
		{
		public:
			WsaNetworkHandler() :
				m_isLastMessageFullyQueued(false),
				m_spBus(nullptr)
			{}

			void onMessageQueued(const char*)
			{
				m_isLastMessageFullyQueued = true;
			}

			void onMessageReceived(const char* msg)
			{
				Message desMessage;
				SerDes::deserializeMessageWSA(&desMessage, msg);

				if (m_spBus != nullptr)
				{
					m_spBus->publish(desMessage);
				}
			}

			bool consumeMessageQueuedFlag()
			{
				bool flag = m_isLastMessageFullyQueued;
				m_isLastMessageFullyQueued = false;
				return flag;
			}

			void setMessageBus(std::shared_ptr<Bus> spBUs)
			{
				m_spBus = std::move(spBUs);
			}

		private:
			bool m_isLastMessageFullyQueued;
			std::shared_ptr<Bus> m_spBus;
		};

		std::shared_ptr<WsaNetworking::WsaClient>& getWsaClientInstance()
		{
			static auto client = std::make_shared<WsaNetworking::WsaClient>();
			return client;
		}

		std::shared_ptr<WsaNetworking::WsaServer>& getWsaServerInstance()
		{
			static auto server = std::make_shared<WsaNetworking::WsaServer>();
			return server;
		}

		const std::shared_ptr<WsaNetworkHandler>& getWsaNetworkHandlerInstance()
		{
			static auto handler = std::make_shared<WsaNetworkHandler>();
			return handler;
		}

		void initializeClient(std::shared_ptr<WsaNetworking::WsaClient>& wsaCLient)
		{
			static bool isInitialized = false;

			if (isInitialized)
			{
				return;
			}

			const auto& pNetworkHandler = getWsaNetworkHandlerInstance();

			pNetworkHandler->setMessageBus(getMessageBusInstance());

			wsaCLient->start();
			wsaCLient->openServerRecvThread<WsaNetworkHandler>(pNetworkHandler.get());

			isInitialized = true;
		}

		void sendFromClient(const Message& message, std::shared_ptr<WsaNetworking::WsaClient>& wsaCLient)
		{
			std::jthread relayThread = std::jthread([&wsaCLient, &message](std::stop_token st)
				{
					char serializedMsg[NetworkFactory::sMaxWsaPayloadLength];
					uint32_t messageSize;

					SerDes::serializeMessage(serializedMsg, message, &messageSize);

					WsaNetworkHandler handler;
					wsaCLient->sendServerMessage<WsaNetworkHandler>(serializedMsg, messageSize, &handler);

					while (!st.stop_requested() && !handler.consumeMessageQueuedFlag())
					{
					};
				}
			);
		}

		const std::shared_ptr<Bus>& getMessageBusInstance()
		{
			static auto bus = std::make_shared<Bus>();
			return bus;
		}

		void initializeClient(NetworkBackend backend)
		{
			switch (backend)
			{
			case Dataflow::NetworkBackend::NetworkBackend_WSA:
				initializeClient(getWsaClientInstance());
				break;
			default:
				break;
			}
		}

		void sendFromClient(const Message& message, NetworkBackend backend)
		{
			switch (backend)
			{
			case Dataflow::NetworkBackend::NetworkBackend_WSA:
				sendFromClient(message, getWsaClientInstance());
				break;
			default:
				break;
			}
		}
	}
}
