#ifndef NNOBSERVERRECEIVER_H
#define NNOBSERVERRECEIVER_H

#include "message.h"
#include "message_bus.h"
#include <functional>
#include <iostream>

#define MAKE_CALLBACK(x) [this](const NNObserver::Message& m) { x(m); }

namespace NNObserver
{
	class MessageReceiver
	{
	public:
		MessageReceiver() : m_linkId(-1), m_pBus(nullptr) {}
		MessageReceiver(Bus* pBus) : m_linkId(-1), m_pBus(pBus) {}
		virtual ~MessageReceiver() 
		{
			if (m_linkId > -1 && m_pBus)
			{
				m_pBus->unsubscribe(m_linkId);

				std::cout << "unsubbed! \n";
			}
		}

	protected:
		int m_linkId;
		Bus* m_pBus;
	};

	class HealthTelemetry : public MessageReceiver
	{
	public:
		HealthTelemetry(Bus* pBus) : MessageReceiver(pBus)
		{
			m_linkId = pBus->subscribe(TopicId::Topic_Hartbeat, MAKE_CALLBACK(onPulseReceived) );
		}
		~HealthTelemetry()
		{
			std::cout << "destroying health telemetry: ";
		}

	private:
		void onPulseReceived(const Message& message)
		{
			std::cout << "Pulse log: "
				<< Topic::getString(message.topicId) << " : "
				<< message.payload << '\n';
		}
	};

	class Display : public MessageReceiver
	{
	public:
		Display(Bus* pBus) : MessageReceiver(pBus)
		{
			m_linkId = pBus->subscribe(TopicId::Topic_CameraFrame, MAKE_CALLBACK(onCameraFrameReceived) );
		}
		~Display()
		{
			std::cout << "destroying display tracker: ";
		}

	private:
		void onCameraFrameReceived(const Message& message)
		{
			std::cout << "Frame: " << message.payload << '\n';
		}
	};

	class CollisionTracker : public MessageReceiver
	{
	public:
		CollisionTracker(Bus* pBus) : MessageReceiver(pBus)
		{
			m_linkId = pBus->subscribe(TopicId::Topic_SensorData, MAKE_CALLBACK(onSensorDataReceived) );
		}
		~CollisionTracker()
		{
			std::cout << "destroying collision tracker: ";
		}

	private:
		void onSensorDataReceived(const Message& message)
		{
			std::cout << "Collision: " << message.payload << '\n';
		}
	};
}

#endif // NNOBSERVERRECEIVER_H