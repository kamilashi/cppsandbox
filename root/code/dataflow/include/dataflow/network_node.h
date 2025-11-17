#ifndef DATAFLOWNETWORKNODE_H
#define DATAFLOWNETWORKNODE_H

#include <vector>
#include <thread>
#include <memory>

#include "dataflow/node.h"

namespace Dataflow
{
	namespace Ipc
	{
		template <Process P>
		class NetworkProcess
		{
		public:
			NetworkProcess()
				: m_process{}
			{ }

			void fire(std::vector<Input>& ins, std::vector<Output>& outs)
			{
				m_process.fire(ins, outs);

				for (auto& out : outs)
				{
					Message msg = out.consume(); //#todo: add isPending check

					m_relay->relay(msg);
				}
			}

			void setClientBusRelay(std::shared_ptr<ClientBusRelay> relay)
			{
				m_relay = std::move(relay);
			}

		private:
			P m_process;                               
			std::shared_ptr<ClientBusRelay> m_relay;       
		};


		template <Process P>
		class NetworkNode
		{
		public:
			NetworkNode(
				float intervalM, 
				std::initializer_list<Input::InitList> inputs,
				std::initializer_list<Output::InitList> outputs,
				std::shared_ptr<ClientBusRelay> spRelay
				) : 
				m_node (intervalM, inputs, outputs, spRelay->getMessageBus())
			{
				m_node.getProcess().setClientBusRelay(spRelay);
			}

		private:
			Node <NetworkProcess<P>> m_node;
		};
	}
}

#endif // DATAFLOWNODE_H