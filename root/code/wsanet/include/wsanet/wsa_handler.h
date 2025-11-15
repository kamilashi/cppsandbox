#ifndef WSANETWORKINGHANDLER_H
#define WSANETWORKINGHANDLER_H

#include <thread>
#include <type_traits>
#include <iostream>

namespace WsaNetworking
{
	template <class T>
	concept ConcreteHandler =
		requires (T h, const char* m)
	{
		h.onMessageQueued(m);
		h.onMessageReceived(m);
	};

	struct DummyHandler
	{
		static void onMessageReceived(const char* message)
		{
			std::cout << "Message received: " << message << std::endl;
		}

		static void onMessageQueued(const char* message)
		{
			std::cout << "Message queued: " << message << std::endl;
		}
	};
}

#endif WSANETWORKINGHANDLER_H