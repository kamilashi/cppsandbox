#include "wsanet/wsa_client.h"

int main(int argc, char* argv[])
{
	WsaNetworking::WsaClient client;
	client.start();

// 	while (getchar() != '\n')
// 	{
// 	};

	//client.requestStop();

	return 0;
}