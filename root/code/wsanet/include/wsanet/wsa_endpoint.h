#ifndef WSANETWORKINGENDPOINT_H
#define WSANETWORKINGENDPOINT_H

#include <winsock2.h>   
#include <ws2tcpip.h>   
#include <windows.h>  
#include <iostream>
#include <mutex>

#include "stdafx.h"
#include "wsanet/wsa_helpers.h"

namespace WsaNetworking
{
	class WsaEndpoint
	{
	public:
		virtual ~WsaEndpoint() = default;

	protected:
		WsaEndpoint() :
			m_initializeResult(ConnectionState::WSACS_UNKNOWN)
		{ }

		ConnectionState initializeWSA();
		ConnectionState createSocket(SOCKET* pSocket);

		ConnectionState m_initializeResult;
	};
}


#endif // WSANETWORKINGENDPOINT_H