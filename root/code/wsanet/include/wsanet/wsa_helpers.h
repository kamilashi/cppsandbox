#ifndef WSANETWORKINGHELPERS_H
#define WSANETWORKINGHELPERS_H

namespace WsaNetworking
{
	enum class ConnectionState
	{
		WSACS_OK,
		WSACS_CLRECVFAIL,
		WSACS_CLSENDFAIL
	};
}


#endif // WSANETWORKINGHELPERS_H