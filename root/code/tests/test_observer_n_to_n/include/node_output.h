#ifndef NNOBSERVERNODEOUTPUT_H
#define NNOBSERVERNODEOUTPUT_H

#include "publisher.h"

namespace NNObserver
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

#endif // NNOBSERVERNODEOUTPUT_H