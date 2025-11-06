#ifndef NNOBSERVERNODEINPUT_H
#define NNOBSERVERNODEINPUT_H

#include "topic.h"
#include "subscriber.h"
#include "message.h"
#include <deque>

namespace NNObserver
{
	class BackpressureHandler 
	{
	public :
		BackpressureHandler () : m_messageQueue{} 
		{}
		void push(const Message& message)
		{
			m_messageQueue.emplace_back(message);
		}

		Message pop()
		{
			Message message = m_messageQueue.front();
			m_messageQueue.pop_front();
			return message;
		}

		size_t getSize() const 
		{
			return m_messageQueue.size();
		}

	private:
		std::deque<Message> m_messageQueue;
	};


	class Input
	{
	public:
		Input(TopicId topic, size_t consumeCount = 1) :
			m_sub(topic),
			m_isReady(false),
			m_consumeCount(consumeCount),
			m_queue{}
		{
		}

		void initialize(std::weak_ptr<Bus> wpBus)
		{
			if (!m_sub.isRegistered())
			{
				m_sub.subscribe(wpBus, MAKE_CALLBACK(onInputReceived));
			}
		}

		void onInputReceived(const Message& message)
		{
			m_queue.push(message);
		}

		const Message consume()
		{
			if (m_queue.getSize() > 0)
			{
				return m_queue.pop();
			}

			// #todo: add error handling
			return Message{TopicId::Topic_Count, "none", "invalid"};
		}

		bool isReady() const
		{
			return m_queue.getSize() >= m_consumeCount;
		}

	private:
		Subscriber m_sub;
		bool m_isReady;

		size_t m_consumeCount;
		BackpressureHandler m_queue;
	};
}
#endif // NNOBSERVERNODEINPUT_H