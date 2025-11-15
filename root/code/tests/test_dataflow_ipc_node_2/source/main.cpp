#include "dataflow/client_bus_relay.h"
#include "dataflow/node.h"

#include <iostream>

namespace Dataflow
{
	class AddNodeIpc : public Node
	{
	public:
		AddNodeIpc(ClientBusRelay* pRelay) :
			Node(500),
			m_pRelay(pRelay)
		{
			m_inputs.reserve(1);
			m_inputs.emplace_back(TopicId::Topic_NumberGen, 2);

			m_outputs.reserve(1);
			m_outputs.emplace_back(TopicId::Topic_Sum);

			Node::connectAndStart(pRelay->getMessageBus());
		}
	private:
		void fire() override
		{
			Message op1 = m_inputs[0].consume();
			Message op2 = m_inputs[0].consume();

			Message sum = Message(
				TopicId::Topic_Sum,
				"add",
				std::format(" {} + {} ", op1.userData, op2.userData));

			sum.userData = op1.userData + op2.userData;

			m_pRelay->relay(sum);
		}

		ClientBusRelay* m_pRelay;
	};

	void printMessage(const Message& message)
	{
		std::cout << message << std::endl;
	}

	int testClient()
	{
		ClientBusRelay clientRelay;

		AddNodeIpc node(&clientRelay);

		Subscriber sumPrinter(TopicId::Topic_Sum);

		sumPrinter.subscribe(clientRelay.getMessageBus(), printMessage);

		while (getchar() != '\n')
		{
		}

		return 0;
	}

	int runTest()
	{
		return testClient();
	}
}


int main(int argc, char* argv[])
{
	return Dataflow::runTest();
}