#ifndef DATAFLOWNODEINPUT_H
#define DATAFLOWNODEINPUT_H

#include "dataflow/topic.h"
#include "dataflow/subscriber.h"
#include "dataflow/message.h"
#include <deque>
#include <mutex>
#include <chrono>

namespace Dataflow
{
	class BackpressureHandler 
	{
	public :
		BackpressureHandler (){}

		void push(const Message& message)
		{
			m_messageQueue.emplace_back(message);
		}

		Message pop()
		{
			Message message = std::move(m_messageQueue.front());
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
		struct InitList
		{
			TopicId topic;
			size_t consumeCount;

			InitList() = delete;

			InitList(TopicId topic) :
				topic(topic), 
				consumeCount(1)
			{ }

			InitList(TopicId topic, size_t consumeCount) : 
				topic(topic),
				consumeCount(consumeCount)
			{ }
		};

		Input(TopicId topic, size_t consumeCount = 1) :
			m_sub(topic),
			m_consumeCount(consumeCount)
		{}

		Input(InitList list) :
			m_sub(list.topic),
			m_consumeCount(list.consumeCount)
		{ }

		Input(Input&& other) noexcept
			: m_sub(std::move(other.m_sub)),
			m_consumeCount(other.m_consumeCount)
		{
			std::lock_guard<std::mutex> lock(other.m_mtx);
			m_queue = std::move(other.m_queue);
		}

		Input& operator=(Input&& other) noexcept 
		{
			if (this != &other) {
				std::scoped_lock lock(m_mtx, other.m_mtx);
				m_sub = std::move(other.m_sub);
				m_queue = std::move(other.m_queue);
				m_consumeCount = other.m_consumeCount;
			}
			return *this;
		}

		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

		void initialize(std::weak_ptr<Bus> wpBus)
		{
			if (!m_sub.isRegistered())
			{
				m_sub.subscribe(wpBus, MAKE_CALLBACK(onInputReceived));
			}
		}

		void onInputReceived(const Message& message)
		{
			std::lock_guard<std::mutex> lock(m_mtx);
			m_queue.push(message);
		}

		Message consume()
		{
			{
				std::lock_guard<std::mutex> lock(m_mtx);

				if (m_queue.getSize() > 0)
				{
					return m_queue.pop();
				}
			}

			// #todo: add error handling
			return Message{};
		}

		bool isReady() const
		{
			std::lock_guard<std::mutex> lock(m_mtx);
			return m_queue.getSize() >= m_consumeCount;
		}

	private:
		Subscriber m_sub;

		size_t m_consumeCount;
		BackpressureHandler m_queue;
		mutable std::mutex m_mtx;
	};
}
#endif // DATAFLOWNODEINPUT_H