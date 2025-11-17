
#include "dataflow/network_factory.h"
#include "dataflow/serdes.h"

#include "wsanet/wsa_client.h"
#include "wsanet/wsa_server.h"
#include "wsanet/wsa_handler.h"


namespace Dataflow
{
	namespace Ipc
	{
		namespace NetworkFactory
		{
			static constexpr uint32_t sMaxWsaPayloadLength = 1024;

			class WsaClientHandler
			{
			public:
				WsaClientHandler() :
					m_isLastMessageFullyQueued(false),
					m_spBus(nullptr)
				{
				}

				void onMessageQueued(const char*)
				{
					m_isLastMessageFullyQueued = true;
				}

				void onMessageReceived(const char* msg)
				{
					Message desMessage;
					SerDes::deserializeMessageWsa(&desMessage, msg);

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

			const std::shared_ptr<WsaClientHandler>& getWsaNetworkHandlerInstance()
			{
				static auto handler = std::make_shared<WsaClientHandler>();
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
				wsaCLient->openServerRecvThread<WsaClientHandler>(pNetworkHandler.get());

				isInitialized = true;
			}

			void sendFromClient(const Message& message, std::shared_ptr<WsaNetworking::WsaClient>& wsaCLient)
			{
				std::jthread relayThread = std::jthread([&wsaCLient, &message](std::stop_token st)
					{
						char serializedMsg[NetworkFactory::sMaxWsaPayloadLength];
						uint32_t messageSize;

						SerDes::serializeMessageWsa(serializedMsg, message, &messageSize);

						WsaClientHandler handler;
						wsaCLient->sendServerMessage<WsaClientHandler>(serializedMsg, messageSize, &handler);

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
				case NetworkBackend::NetworkBackend_WSA:
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
				case NetworkBackend::NetworkBackend_WSA:
					sendFromClient(message, getWsaClientInstance());
					break;
				default:
					break;
				}
			}
		}
	}
}
