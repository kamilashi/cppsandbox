#ifndef WSANETWORKINGHELPERS_H
#define WSANETWORKINGHELPERS_H

#include <winsock2.h>   
#include <ws2tcpip.h>   
#include <windows.h>  

#include <mutex>  

#include "wsa_handler.h"

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
		static constexpr uint32_t sPrependLength = sizeof(uint32_t);
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
		if (sError == 0 ||
			sError == WSAEWOULDBLOCK ||
			sError == WSAEINTR ||
			sError == WSAENOBUFS)
		{
			return false;
		}

		return true;
	}

	template<ConcreteHandler H>
	void onMessageReceived(const char* message, H* pHandler = nullptr)
	{
		if (pHandler != nullptr)
		{
			pHandler->onMessageReceived(message);
		}
		else
		{
			static H handler;
			handler.onMessageReceived(message);
		}
	}

	template<ConcreteHandler H>
	void onMessageSent(const char* message, H* pHandler = nullptr)
	{
		if (pHandler != nullptr)
		{
			pHandler->onMessageQueued(message);
		}
		else
		{
			static H handler;
			handler.onMessageQueued(message);
		}
	}

	FORCEINLINE void storeHostu32(char* out32, uint32_t in32)
	{
		uint32_t hostOrderValue = htonl(in32);
		memcpy(out32, &hostOrderValue, WsaMessageFrame::sPrependLength);
	}

	FORCEINLINE void storeHostf32(char* out32, float in32)
	{
		static_assert(sizeof(float) == WsaMessageFrame::sPrependLength, "float must be 32 bit");
		uint32_t bits = 0;
		memcpy(&bits, &in32, WsaMessageFrame::sPrependLength);
		storeHostu32(out32, bits);
	}

	FORCEINLINE uint32_t readHostu32(const char* in32)
	{
		uint32_t value_net;
		memcpy(&value_net, in32, WsaMessageFrame::sPrependLength);
		return ntohl(value_net); 
	}

	FORCEINLINE float readHostf32(const char* in32)
	{
		uint32_t bits = readHostu32(in32);

		float out32;
		std::memcpy(&out32, &bits, WsaMessageFrame::sPrependLength);
		return out32;
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

			if (justSentByteCount <= 0 && isFatalError())
			{
				return ConnectionState::WSACS_SENDFAIL;
			}

			sentByteCount += justSentByteCount;
		} while (sentByteCount < messageLength);

		return ConnectionState::WSACS_OK;
	}

	FORCEINLINE ConnectionState sendMessageFrame(SOCKET* pSocket, std::mutex* pMutex, const char* payload, uint32_t payloadLength)
	{
		const uint32_t payloadSize = payloadLength + 1; 
		const uint32_t fullSize = payloadSize + WsaMessageFrame::sPrependLength;

		char prepend[WsaMessageFrame::sPrependLength];

		storeHostu32(prepend, payloadSize);

		WsaMessageFrame frame;
		frame.buffer = new char[fullSize];

		memcpy(frame.buffer, prepend, WsaMessageFrame::sPrependLength);
		memcpy(frame.buffer + WsaMessageFrame::sPrependLength, payload, payloadLength);
		frame.buffer[fullSize - 1] = '\0';

		ConnectionState state = sendMessage(pSocket, pMutex, frame.buffer, fullSize);

		return state;
	}

	FORCEINLINE ConnectionState readMessage(SOCKET* pSocket, char* message, uint32_t messageLength)
	{
		SOCKET& clientSocket = *pSocket;

		int recvdByteCount = 0;
		do
		{
			int justRecvdByteCount = recv(clientSocket, message + recvdByteCount, messageLength - recvdByteCount, 0);
			if (justRecvdByteCount <= 0 && isFatalError())
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