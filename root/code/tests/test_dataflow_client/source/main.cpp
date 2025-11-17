#include "dataflow/client_bus_relay.h"

#include <iostream>

namespace Dataflow
{
	int testClient()
	{
		ClientBusRelay clientRelay(NetworkBackend::NetworkBackend_WSA);
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
		return testClient();
	}
}


int main(int argc, char* argv[])
{
	return Dataflow::runTest();
}