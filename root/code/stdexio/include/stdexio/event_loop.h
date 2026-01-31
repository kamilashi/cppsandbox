#ifndef STDEXIOEVENTLOOP_H
#define STDEXIOEVENTLOOP_H

#include <thread>
#include <atomic>
#include <iostream>

#include <stdexec/execution.hpp>
#include <exec/async_scope.hpp>
#include <unordered_map>

#include "stdexio/event_handler.h"

namespace stdexio
{
	template <class Scheduler>
	class EventLoop
	{
	public:
		EventLoop(Scheduler sched) : 
			m_scheduler(std::move(sched)), 
			m_isRunning{ true }
		{
			start();
		}

		~EventLoop()
		{
			stop();
		}

		void addEventHandler(const char eventKey, std::function<void()> handler)
		{
			if (m_eventHandlerMap.find(eventKey) == m_eventHandlerMap.end())
			{
				m_eventHandlerMap[eventKey] = std::move(handler); // #TODO: enable duplicate handlers!
			}
			else
			{
				std::cout << "\nKey is already assigned!\n\n";
			}
		}

		void requestStop()
		{
			stop();
		}

		bool isRunning()
		{
			return m_isRunning.load(std::memory_order_acquire);
		}

	private:
		Scheduler m_scheduler;
		exec::async_scope m_scope;
		std::jthread m_eventProcessThread;
		std::atomic<bool> m_isRunning;
		std::unordered_map<char, std::function<void()>>	  m_eventHandlerMap; 

		void postEvent(std::function<void()> fn)
		{
			m_scope.spawn(
				stdexec::schedule(m_scheduler)
				| stdexec::then([fn = std::move(fn)]() mutable { fn(); })
			);
		}

		void start()
		{
			m_eventProcessThread = std::jthread([this](std::stop_token stopToken/*unused*/)
			{
				while (!stopToken.stop_requested())
				{
					int key = std::getchar(); // keyboard input

					auto it = m_eventHandlerMap.find(key);

					if (it != m_eventHandlerMap.end())
					{
						std::function<void()> handler = it->second; // copy
						postEvent(std::move(handler));
					}
				}

				m_isRunning.store(false, std::memory_order_release);
			});
		}

		void stop()
		{
			m_eventProcessThread.request_stop();
			stdexec::sync_wait(m_scope.on_empty());
		}
	};
}


#endif // STDEXIOEVENTLOOP_H