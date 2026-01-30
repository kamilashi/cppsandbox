#ifndef STDEXIOEVENTLOOP_H
#define STDEXIOEVENTLOOP_H

#include <thread>
#include <stdexec/execution.hpp>
#include <exec/async_scope.hpp>
#include <unordered_map>
#include <unordered_set>

#include "stdexio/event_handler.h"

namespace stdexio
{
	template <class Scheduler>
	class EventLoop
	{
	public:
		template <class Scheduler>
		EventLoop(Scheduler sched) : m_scheduler(std::move(sched))
		{
			m_eventProcessThread = std::jthread([&](std::stop_token /*unused*/) 
			{
				int ch = std::getchar(); // keyboard input

				auto it = m_eventHandlerMap.find(key);

				if (it == m_eventHandlerMap.end())
				{ 
					return;
				}

				std::function<void()> handler = it->second; // copy
				postEvent(std::move(handler));
			});
		}

		~EventLoop()
		{
			stdexec::sync_wait(m_scope.on_empty());
			m_eventProcessThread.request_stop();
		}

		void addEventHandler(char eventKey, std::function<void()> handler)
		{
			if (m_eventHandlerMap.find(eventKey) != m_eventHandlerMap.end())
			{
				m_eventHandlerMap.at(eventKey) = std::move(handler); // #TODO: enable duplicate handlers!
			}
		}

	private:
		Scheduler m_scheduler;
		exec::async_scope m_scope;
		std::jthread m_eventProcessThread;
		std::unordered_map<char, std::function<void()>>	  m_eventHandlerMap; 

		void postEvent(std::function<void()> fn)
		{
			m_scope.spawn(
				stdexec::schedule(m_scheduler)
				| stdexec::then([fn = std::move(fn)]() mutable { fn(); })
			);
		}
	};
}


#endif // STDEXIOEVENTLOOP_H