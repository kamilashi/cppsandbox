#ifndef WSANETWORKINGHELPERS_H
#define WSANETWORKINGHELPERS_H

namespace WsaNetworking
{
	enum class ConnectionState
	{
		WSACS_OK,
		WSACS_INITFAIL,
		WSACS_RECVFAIL,
		WSACS_SENDFAIL,
		WSACS_UNKNOWN
	};

	struct WsaMessageFrame
	{
		static constexpr size_t sPrependLength = sizeof(uint32_t);
		char* buffer = nullptr;
		ConnectionState state = ConnectionState::WSACS_UNKNOWN;

		WsaMessageFrame() = default;

		~WsaMessageFrame()
		{
			if (buffer != nullptr)
			{
				delete[] buffer;
			}
		}
	};

	FORCEINLINE bool isFatalError()
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

	FORCEINLINE void storeHostu32(char* out32, uint32_t in32)
	{
		uint32_t hostOrderValue = htonl(in32);
		memcpy(out32, &hostOrderValue, WsaMessageFrame::sPrependLength);
	}

	FORCEINLINE uint32_t readHostu32(const char* in32)
	{
		uint32_t value_net;
		memcpy(&value_net, in32, WsaMessageFrame::sPrependLength);
		return ntohl(value_net); 
	}

	FORCEINLINE ConnectionState sendMessage(SOCKET* pSocket, std::mutex* pMutex, const char* message, size_t messageLength)
	{
		int justSentByteCount = 0;
		int sentByteCount = 0;

		do
		{
			{
				std::lock_guard<std::mutex> lock(*pMutex);

				SOCKET& clientSocket = *pSocket;
				justSentByteCount = send(clientSocket, message + sentByteCount, messageLength - sentByteCount, 0);
			}

			if (justSentByteCount <= 0)
			{
				return ConnectionState::WSACS_SENDFAIL;
			}

			sentByteCount += justSentByteCount;
		} while (sentByteCount < messageLength);

		return ConnectionState::WSACS_OK;
	}

	FORCEINLINE ConnectionState sendMessageFrame(SOCKET* pSocket, std::mutex* pMutex, const char* payload)
	{
		const size_t payloadLength = strlen(payload) + 1;
		const size_t fullLength = payloadLength + WsaMessageFrame::sPrependLength;

		char prepend[WsaMessageFrame::sPrependLength];

		storeHostu32(prepend, static_cast<uint32_t> (payloadLength));

		WsaMessageFrame frame;
		frame.buffer = new char[fullLength];

		memcpy(frame.buffer, prepend, WsaMessageFrame::sPrependLength);
		strcpy(frame.buffer + WsaMessageFrame::sPrependLength, payload); // end with null termination

		ConnectionState state = sendMessage(pSocket, pMutex, frame.buffer, fullLength);

		return state;
	}

	FORCEINLINE ConnectionState readMessage(SOCKET* pSocket, char* message, size_t messageLength)
	{
		SOCKET& clientSocket = *pSocket;

		int recvdByteCount = 0;
		do
		{
			int justRecvdByteCount = recv(clientSocket, message + recvdByteCount, messageLength - recvdByteCount, 0);
			if (justRecvdByteCount <= 0)
			{
				return ConnectionState::WSACS_RECVFAIL;
			}

			recvdByteCount += justRecvdByteCount;
		} while (recvdByteCount < messageLength);

		return ConnectionState::WSACS_OK;
	}

	FORCEINLINE WsaMessageFrame getMessageFrame(SOCKET* pSocket)
	{
		char prepend[WsaMessageFrame::sPrependLength];

		WsaMessageFrame frame;
		frame.state = readMessage(pSocket, prepend, WsaMessageFrame::sPrependLength);

		if (frame.state != ConnectionState::WSACS_OK)
		{
			return frame;
		}

		uint32_t payloadLength = readHostu32(prepend);

		frame.buffer = new char[payloadLength];
		frame.state = readMessage(pSocket, frame.buffer, payloadLength);

		return frame;
	}
}


#endif // WSANETWORKINGHELPERS_H