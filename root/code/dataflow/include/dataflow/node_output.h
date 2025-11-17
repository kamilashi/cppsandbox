#ifndef DATAFLOWNODEOUTPUT_H
#define DATAFLOWNODEOUTPUT_H

#include "dataflow/publisher.h"
#include <mutex>

namespace Dataflow
{
	class Output
	{
	public:
		struct InitList
		{
			TopicId topic;

			InitList() = delete;

			InitList(TopicId topic) : topic(topic) {}
		};

		Output(TopicId topic) : m_pub(topic)
		{ }

		Output(InitList initList) : m_pub(initList.topic)
		{ }

		Output(Output&& other) noexcept
			: m_pub(std::move(other.m_pub)),
			  m_message(std::move(other.m_message))
		{ }

		Output& operator=(Output&& other) noexcept
		{
			if (this != &other) {
				std::scoped_lock lock(m_mutex, other.m_mutex);
				m_pub = std::move(other.m_pub);
				m_message = std::move(other.m_message);
			}
			return *this;
		}

		Output(const Output&) = delete;
		Output& operator=(const Output&) = delete;

		void initialize(std::weak_ptr<Bus> wpBus)
		{
			if (!m_pub.isRegistered())
			{
				m_pub.registerPublisher(wpBus);
			}
		}

		void produce(const Message& message) 
		{
			m_pub.tryPublish(message);
		}

		void store(Message message)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_message = std::move(message); //#todo: optimize
		}

		Message consume()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			Message message{ m_message };
			m_message.reset();
			return message; //#todo: optimize
		}

	private:
		Publisher m_pub;
		Message m_message;
		std::mutex m_mutex;
	};
}

#endif // DATAFLOWNODEOUTPUT_H