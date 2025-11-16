#ifndef WSANETWORKINGHANDLER_H
#define WSANETWORKINGHANDLER_H

#include <thread>
#include <type_traits>
#include <iostream>

namespace WsaNetworking
{
	template <class T>
	concept ConcreteHandler =
		std::default_initializable<T> &&
		requires (T h, const char* m)
	{
		h.onMessageQueued(m);
		h.onMessageReceived(m);
	};

	struct DummyHandler
	{
		void onMessageReceived(const char* message)
		{
			std::cout << "Message received: " << message << std::endl;
		}

		void onMessageQueued(const char* message)
		{
			std::cout << "Message queued: " << message << std::endl;
		}
	};
}

#endif