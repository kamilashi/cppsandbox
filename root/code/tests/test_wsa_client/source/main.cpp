#include "wsanet/wsa_client.h"

int main(int argc, char* argv[])
{
	WsaNetworking::WsaClient client;
	client.start();

	std::jthread broadcastThread = std::jthread([&](std::stop_token st)
		{
			while (!st.stop_requested())
			{
				client.sendDummyMessage();
				std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(1000));
			}
		});

	while (getchar() != '\n')
	{
	};

	client.requestStop();

	return 0;
}