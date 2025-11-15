#include "build_config.h"
#include "dataflow/client_bus_relay.h"
#include "dataflow/serdes.h"

#include <iostream>

namespace Dataflow
{
	int testSerDes()
	{
		constexpr size_t maxSerializedMessgeSize = 1024;

		Message messageIn(TopicId::Topic_NumberGen, "test source", "3.125f");
		messageIn.userData = 3.125f;

		char payload[maxSerializedMessgeSize];

		SerDes::serializeMessage(payload, messageIn);

		Message messageOut;

		SerDes::deserializeMessage(&messageOut, payload);

		if (messageOut != messageIn)
		{
			std::cout << "Test failed!" << std::endl << std::endl;

			std::cout << "In message: " << std::endl;
			std::cout << messageIn << std::endl << std::endl;

			std::cout << "Out message: " << std::endl;
			std::cout << messageOut << std::endl << std::endl;

			return -1;
		}

		return 0;
	}

	int testClient()
	{
		ClientBusRelay clientRelay;
		Message messageIn(TopicId::Topic_NumberGen, "test source", "3.125f");
		messageIn.userData = 3.125f;
		clientRelay.relay(messageIn);

		while (getchar() != '\n')
		{
		}

		return 0;
	}

	int runTest()
	{
		//return testSerDes();
		return testClient();
	}
}


int main(int argc, char* argv[])
{
	return Dataflow::runTest();
}