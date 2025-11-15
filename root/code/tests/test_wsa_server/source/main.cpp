#include <iostream>
#include <thread>
#include <chrono>
#include "wsanet/wsa_server.h"

struct ServerHandler
{
	void onMessageReceived(const char* message)
	{
		std::cout << "Message received from client: " << message << std::endl;
	}

	void onMessageQueued(const char* message)
	{
		std::cout << "message queued to client: " << message << std::endl;
	}
};

int main(int argc, char* argv[])
{
	WsaNetworking::WsaServer server;
	server.start();
	server.acceptNewClients<ServerHandler>();

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