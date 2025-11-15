#include "dataflow/client_bus_relay.h"
#include "dataflow/node.h"

#include <iostream>

namespace Dataflow
{
	class NumberGenNodeIpc : public Node
	{
	public:
		NumberGenNodeIpc(ClientBusRelay* pRelay) :
			Node(500),
			m_nextNumber(0),
			m_pRelay(pRelay)
		{
			m_outputs.reserve(1);
			m_outputs.emplace_back(TopicId::Topic_NumberGen);

			Node::connectAndStart(pRelay->getMessageBus());
		}
	private:
		void run() override
		{
			Message out = Message(
				TopicId::Topic_NumberGen,
				"number_gen",
				std::format("{}", m_nextNumber));
			out.userData = m_nextNumber;

			std::cout << out << std::endl;

			m_pRelay->relay(out);

			m_nextNumber++;
		}

		float m_nextNumber;
		ClientBusRelay* m_pRelay;
	};

	int testClient()
	{
		ClientBusRelay clientRelay;

		NumberGenNodeIpc numGen(&clientRelay);

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