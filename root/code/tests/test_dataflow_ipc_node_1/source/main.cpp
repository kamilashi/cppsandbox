#include "dataflow/client_bus_relay.h"
#include "dataflow/node_legacy.h"

#include <iostream>

namespace Dataflow
{
	class NumberGenNodeIpc : public BaseNodeLegacy
	{
	public:
		NumberGenNodeIpc(Ipc::ClientBusRelay* pRelay) :
			BaseNodeLegacy(500),
			m_nextNumber(0),
			m_pRelay(pRelay)
		{
			m_outputs.reserve(1);
			m_outputs.emplace_back(TopicId::Topic_NumberGen);

			BaseNodeLegacy::connectAndStart(pRelay->getMessageBus());
		}
	private:
		void fire() override
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
		Ipc::ClientBusRelay* m_pRelay;
	};

	int testClient()
	{
		Ipc::ClientBusRelay clientRelay(Ipc::NetworkBackend::NetworkBackend_WSA);

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