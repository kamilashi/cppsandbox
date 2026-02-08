#ifndef STDEXIOEVENTLOOP_H
#define STDEXIOEVENTLOOP_H

#include <thread>
#include <atomic>
#include <iostream>

#include <stdexec/execution.hpp>
#include <exec/async_scope.hpp>
#include <unordered_map>
#include <unordered_set>

#include "stdexio/event_handler.h"

namespace stdexio
{
	namespace detail
	{
		template<class Scheduler>
		auto makeSender(Scheduler& sched, std::function<void()> fn)
		{
			return stdexec::schedule(sched)
				| stdexec::then([fn = std::move(fn)]() mutable { fn(); });
		}

		template<class Scheduler>
		using SenderT = decltype(
			makeSender(std::declval<Scheduler&>(),
				std::declval<std::function<void()>>())
			);
	}

	template <class Scheduler>
	class EventLoop
	{
	public:
		EventLoop(Scheduler sched) : 
			m_scheduler(std::move(sched)) 
		{
			start();
		}

		~EventLoop()
		{
			cleanup();
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

		void registerStopKey(const char eventKey)
		{
			if (m_stopEvents.find(eventKey) == m_stopEvents.end())
			{
				m_stopEvents.emplace(eventKey);
			}
		}

		void stop()
		{
			cleanup();
		}

		bool isRunning() const
		{
			return m_stopSender.has_value();
		}

		const auto& getScheduler() const
		{
			return m_scheduler;
		}

		const auto onThreadFinished() const 
		{
			return m_stopSender.value();
		}

		void blockUntilExited() const
		{
			stdexec::sync_wait(onThreadFinished());
			//stdexec::sync_wait(m_scope.on_empty());
		}

	private:

		Scheduler m_scheduler;
		exec::async_scope m_scope;
		std::jthread m_eventProcessThread;
		std::unordered_map<char, std::function<void()>>	  m_eventHandlerMap; 
		std::unordered_set<char> m_stopEvents;

		using SenderT = detail::SenderT<Scheduler>;
		std::optional<SenderT> m_stopSender;

		void postEvent(SenderT sender)
		{
			m_scope.spawn(std::move(sender));
		}

		void start()
		{
			m_stopSender.emplace(detail::makeSender(m_scheduler, [this] { cleanup(); }));

			m_eventProcessThread = std::jthread([this](std::stop_token stopToken /*unused*/)
			{
				while (true)
				{
					int key = std::getchar(); // keyboard input

					if (m_stopEvents.empty())
					{
						std::cout << "\nNo io-engine stop events were registered!" 
  								  << "\nPlease, register one with registerStopEvent() \n\n";
					}
					else if (m_stopEvents.find(key) != m_stopEvents.end())
					{
						break;
					}

					auto it = m_eventHandlerMap.find(key);

					if (it != m_eventHandlerMap.end())
					{
						std::function<void()> handler = it->second; // copy
						postEvent(detail::makeSender(m_scheduler, std::move(handler)));
					}
				}

				postEvent(m_stopSender.value());
				m_stopSender.reset();
			});
		}

		void cleanup()
		{
			if (m_eventProcessThread.joinable())
			{
				m_eventProcessThread.join();
			}
		}
	};

	template <class Scheduler>
	void blockUntilExited(const EventLoop<Scheduler>& loop)
	{
		if (!loop.isRunning())
		{
			std::cout << "IO thread already finished, won't block.";
			return;
		}

		loop.blockUntilExited();
	}
}


#endif // STDEXIOEVENTLOOP_H