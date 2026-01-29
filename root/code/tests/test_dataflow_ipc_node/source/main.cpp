#include "dataflow/client_bus_relay.h"
#include "dataflow/network_node.h"

#include <iostream>

namespace Dataflow
{
	struct AddProcess
	{
		void fire(std::vector<Input>& inputs, std::vector<Output>& outputs)
		{
			Message op1 = inputs[0].consume();
			Message op2 = inputs[0].consume();

			Message sum = Message(
				TopicId::Topic_Sum,
				"add",
				std::format(" {} + {} ", op1.userData, op2.userData));

			sum.userData = op1.userData + op2.userData;
			
			outputs[0].stage(sum);
		}
	};

	void printMessage(const Message& message)
	{
		std::cout << message << std::endl;
	}

	int testClient()
	{
		auto spClientRelay = std::make_shared<Ipc::ClientBusRelay>(Ipc::NetworkBackend::NetworkBackend_WSA);

		Ipc::NetworkNode<AddProcess> node(500.0f, 
			{ { TopicId::Topic_NumberGen , 2 } }, 
			{ TopicId::Topic_Sum }, 
			spClientRelay);

		Subscriber sumPrinter(TopicId::Topic_Sum);

		sumPrinter.subscribe(spClientRelay->getMessageBus(), printMessage);

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