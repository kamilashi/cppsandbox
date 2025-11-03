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
			m_topic(topic)
		{}

		~Publisher()
		{
			unregisterPublisher();
		}

		void registerPublisher(std::weak_ptr<Bus> wpBus)
		{
			m_wpBus = std::move(wpBus);

			auto sharedBus = m_wpBus.lock();
			if (sharedBus)
			{
				sharedBus->registerPublisher(m_topic); // #wip
			}

			// #todo: add error handling
		}

		void unregisterPublisher()
		{
			auto sharedBus = m_wpBus.lock();
			if (sharedBus)
			{
				sharedBus->unregisterPublisher(m_topic); // #wip
			}

			// #todo: add error handling
		}

		void tryPublish(std::string_view nodeId, std::string_view payload)
		{
			auto sharedBus = m_wpBus.lock();
			if (sharedBus)
			{
				Message message(m_topic, nodeId, payload);
				sharedBus->publish(message);
			}

			// #todo: add error handling
		}

	protected:
		std::weak_ptr<Bus> m_wpBus;
		TopicId m_topic;
	};
}

#endif // NNOBSERVERPUBLISHER_H