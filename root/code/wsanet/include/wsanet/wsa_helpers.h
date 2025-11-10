#ifndef WSANETWORKINGHELPERS_H
#define WSANETWORKINGHELPERS_H

namespace WsaNetworking
{
	enum class ConnectionState
	{
		WSACS_OK,
		WSACS_INITFAIL,
		WSACS_RECVFAIL,
		WSACS_SENDFAIL
	};

	FORCEINLINE bool shouldStopClient()
	{
		int sError = WSAGetLastError();
		if (sError == WSAEWOULDBLOCK ||
			sError == WSAEINTR ||
			sError == WSAENOBUFS)
		{
			return false;
		}

		return true;
	}
}


#endif // WSANETWORKINGHELPERS_H