
#include "dataflow/network_factory.h"
#include "dataflow/serdes.h"

#include "wsanet/wsa_client.h"
#include "wsanet/wsa_server.h"
#include "wsanet/wsa_handler.h"

#include <atomic>


namespace Dataflow
{
	namespace Ipc
	{
		namespace NetworkFactory
		{
			static constexpr uint32_t sMaxWsaPayloadLength = 1024;
			static std::atomic<bool> isClientInitialized(false);

			class WsaClientHandler
			{
			public:
				WsaClientHandler() :
					m_isLastMessageFullyQueued{ false },
					m_spBus(nullptr)
				{}

				void onMessageReceived(const char* msg)
				{
					Message desMessage;
					SerDes::deserializeMessageWsa(&desMessage, msg);

					if (m_spBus != nullptr)
					{
						m_spBus->publish(desMessage);
					}
				}

				void onMessageQueued(const char*)
				{
					m_isLastMessageFullyQueued.store(true, 
						std::memory_order_release);
				}

				bool consumeMessageQueuedFlag()
				{
					return m_isLastMessageFullyQueued.exchange(false, std::memory_order_acq_rel);
				}

				void setMessageBus(std::shared_ptr<Bus> spBUs)
				{
					m_spBus = std::move(spBUs);
				}

			private:
				std::atomic<bool> m_isLastMessageFullyQueued;
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
				if (isClientInitialized.exchange(true, std::memory_order_acq_rel))
				{
					return;
				}

				const auto& pNetworkHandler = getWsaNetworkHandlerInstance();

				pNetworkHandler->setMessageBus(getMessageBusInstance());

				wsaCLient->start();
				wsaCLient->openServerRecvThread<WsaClientHandler>(pNetworkHandler.get());
			}

			void sendFromClient(const Message& message, std::shared_ptr<WsaNetworking::WsaClient>& wsaCLient)
			{
				char serializedMsg[NetworkFactory::sMaxWsaPayloadLength];
				uint32_t messageSize;

				SerDes::serializeMessageWsa(serializedMsg, message, &messageSize);

				WsaClientHandler handler;
				wsaCLient->sendServerMessage<WsaClientHandler>(serializedMsg, messageSize, &handler);

				while (!handler.consumeMessageQueuedFlag())
				{
					std::this_thread::yield();
				};
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
