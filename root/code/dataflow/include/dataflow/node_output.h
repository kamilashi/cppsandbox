#ifndef DATAFLOWNODEOUTPUT_H
#define DATAFLOWNODEOUTPUT_H

#include "dataflow/publisher.h"

namespace Dataflow
{
	class Output
	{
	public:
		Output(TopicId topic) : m_pub(topic)
		{ }

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

	private:
		Publisher m_pub;
	};
}

#endif // DATAFLOWNODEOUTPUT_H