#include "wsanet/wsa_client.h"

//#define TRACKMALLOC
//#include "malloc_tracker.h"

struct ClientHandler
{
	void onMessageReceived(const char* message)
	{
		std::cout << "Message received from server: " << message << std::endl;
	}

	void onMessageQueued(const char* message)
	{
		std::cout << "Message queued to server: " << message << std::endl;
	}
};

int main(int argc, char* argv[])
{
	WsaNetworking::WsaClient client;
	client.start();
	client.openServerRecvThread<ClientHandler>();

	std::jthread broadcastThread = std::jthread([&](std::stop_token st)
		{
			while (!st.stop_requested())
			{
				client.sendDummyMessage();
				std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(2000));
			}
		});

	while (getchar() != '\n')
	{
	};

	client.requestStop();

	//MallocTracker::printMemoryAllocationMetrics();

	return 0;
}