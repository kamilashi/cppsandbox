#ifndef DATAFLOWNODE_H
#define DATAFLOWNODE_H

#include "dataflow/base_node.h"
#include "dataflow/message_bus.h"

namespace Dataflow
{
	namespace
	{
		template <Dataflow::Process P>
		class LocalProcess
		{
		public:
			LocalProcess()
				: m_process{}
			{  }

			void fire(std::vector<Dataflow::Input>& ins, std::vector<Dataflow::Output>& outs)
			{
				m_process.fire(ins, outs);

				for (auto& out : outs)
				{
					Dataflow::Message msg = out.consume(); //#todo: add isPending check

					outs[0].produce(msg);
				}
			}

		private:
			P m_process;
		};
	}

	template <Process P>
	class Node
	{
	public:
		Node(
			float intervalM,
			std::initializer_list<Input::InitList> inputs,
			std::initializer_list<Output::InitList> outputs,
			std::weak_ptr<Bus> wpBus
		) :
			m_node(intervalM, inputs, outputs, wpBus)
		{ }

	private:
		BaseNode <LocalProcess<P>> m_node;
	};

}

#endif // DATAFLOWNODE_H
