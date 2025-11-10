#include "wsanet/wsa_endpoint.h"

namespace WsaNetworking
{
	ConnectionState WsaEndpoint::initializeWSA()
	{
		WSADATA wsadata;
		int wsaerr;
		WORD wVersionRequested = MAKEWORD(2, 2);
		wsaerr = WSAStartup(wVersionRequested, &wsadata);
		if (wsaerr != 0)
		{
			std::cout << "The winsock dll not found" << std::endl;
			return ConnectionState::WSACS_INITFAIL;
		}

		std::cout << "The winsock dll found" << endl;
		std::cout << "Status: " << wsadata.szSystemStatus << std::endl;

		return ConnectionState::WSACS_OK;
	}

	ConnectionState WsaEndpoint::createSocket(SOCKET* pSocket)
	{
		*pSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (*pSocket == INVALID_SOCKET)
		{
			std::cout << "Error at socket():" << WSAGetLastError() << std::endl;
			return ConnectionState::WSACS_INITFAIL;
		}

		std::cout << "Socket is OK" << std::endl;
		return ConnectionState::WSACS_OK;
	}
}

