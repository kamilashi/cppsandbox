#ifndef NNOBSERVERSUBSCRIBER_H
#define NNOBSERVERSUBSCRIBER_H

#include <memory>
#include "message_bus.h"
	
namespace NNObserver
{
	class Subscriber
	{
	public:
		Subscriber() : m_linkId(-1), m_topic(TopicId::Topic_Count) {}
		Subscriber(TopicId topic) : m_linkId(-1), m_topic(topic) {}
		virtual ~Subscriber()
		{
			unsubscribe();
		}

		void subscribe(std::weak_ptr<Bus> wpBus, Bus::OnMessageCallback callback)
		{
			m_wpBus = std::move(wpBus);

			auto sharedBus = m_wpBus.lock();
			if (sharedBus)
			{
				m_linkId = sharedBus->subscribe(m_topic, callback);
			}
		}

		void unsubscribe()
		{
			auto sharedBus = m_wpBus.lock();
			if (m_linkId > -1 && sharedBus)
			{
				sharedBus->unsubscribe(m_linkId);
			}
		}

	protected:
		int m_linkId;
		std::weak_ptr<Bus> m_wpBus;
		TopicId m_topic;
	};
}

#endif // NNOBSERVERSUBSCRIBER_H