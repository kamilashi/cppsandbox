#ifndef NNOBSERVERPUBLISHER_H
#define NNOBSERVERPUBLISHER_H

#include <string>
#include <memory>
#include <format>

#include "message_bus.h"
#include "message.h"
	
namespace NNObserver
{
	class Publisher
	{
	public:
		Publisher(TopicId topic) : 
			m_topic(topic),
			m_isRegistered(false)
		{}

		~Publisher()
		{
			unregisterPublisher();
		}

		void registerPublisher(std::weak_ptr<Bus> wpBus)
		{
			m_wpBus = std::move(wpBus);

			auto sharedBus = m_wpBus.lock();
			if (!m_isRegistered && sharedBus)
			{
				sharedBus->registerPublisher(m_topic); // #wip
				m_isRegistered = true;
			}

			// #todo: signal unavailable bus
		}

		void unregisterPublisher()
		{
			auto sharedBus = m_wpBus.lock();
			if (m_isRegistered && sharedBus)
			{
				sharedBus->unregisterPublisher(m_topic); // #wip
			}

			m_isRegistered = false;

			// #todo: signal unavailable bus
		}

		void tryPublish(std::string_view nodeId, std::string_view payload) const
		{
			auto sharedBus = m_wpBus.lock();
			if (sharedBus)
			{
				Message message(m_topic, nodeId, payload);
				sharedBus->publish(message);
			}

			// #todo: add error handling
		}

		void tryPublish(const Message& message) const
		{
			auto sharedBus = m_wpBus.lock();
			if (sharedBus)
			{
				sharedBus->publish(message); // copy
			}

			// #todo: add error handling
		}

		bool isRegistered() const
		{
			return m_isRegistered;
		}

	protected:
		std::weak_ptr<Bus> m_wpBus;
		TopicId m_topic;
		bool m_isRegistered;
	};
}

#endif // NNOBSERVERPUBLISHER_H