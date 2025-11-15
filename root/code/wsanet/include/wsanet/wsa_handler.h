#ifndef WSANETWORKINGHANDLER_H
#define WSANETWORKINGHANDLER_H

#include <thread>
#include <type_traits>

namespace WsaNetworking
{
	template <class T>
	concept ConcreteHandler =
		//T{} &&
		std::movable<T> &&
		std::copyable<T> &&
		requires (T h, const char* m)
	{
		h.onMessageQueued(m);
		h.onMessageReceived(m);
	};

	template <ConcreteHandler T>
	class WsaHandler
	{
	public:
		WsaHandler(T handler) : m_handler(std::move(handler)) {};
		//WsaHandler(nullptr) {};
		~WsaHandler() {};

		void onMessageQueued(const char* message)
		{
			m_handler.onMessageQueued(message);
		}

		void onMessageReceived(const char* message)
		{
			m_handler.onMessageReceived(message);
		}

	private:
		T m_handler;
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

#endif WSANETWORKINGHANDLER_H