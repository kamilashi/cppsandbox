#include <iostream>
#include <thread>
#include <chrono>
#include "wsanet/wsa_server.h"


int main(int argc, char* argv[])
{
	WsaNetworking::WsaServer server;
	server.start();
	server.acceptNewClients<WsaNetworking::DummyHandler>();

	std::jthread broadcastThread= std::jthread([&](std::stop_token st)
	{
		while (!st.stop_requested())
		{
			server.broadcastDummyMessage();
			std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(1000));
		}
	});

	while (getchar() != '\n')
	{};

	return 0;
}