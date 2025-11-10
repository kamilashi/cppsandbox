#ifndef WSANETWORKINGENDPOINT_H
#define WSANETWORKINGENDPOINT_H

#include <winsock2.h>   
#include <ws2tcpip.h>   
#include <windows.h>  
#include <iostream>
#include <thread>
#include <mutex>

#include "stdafx.h"
#include "wsanet/wsa_helpers.h"

namespace WsaNetworking
{
	class WsaEndpoint
	{
	protected:
		ConnectionState initializeWSA();
		ConnectionState createSocket(SOCKET* pSocket);
	};
}


#endif // WSANETWORKINGENDPOINT_H