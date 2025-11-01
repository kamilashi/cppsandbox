#ifndef NNOBSERVERRECEIVER_H
#define NNOBSERVERRECEIVER_H
#include "node.h"

namespace NNObserver
{
	class HealthMonitor : public Node
	{
	public:
		HealthMonitor(std::weak_ptr<Bus> wpBus) : Node(wpBus)
		{
			auto sharedBus = m_wpBus.lock();
			if (sharedBus)
			{
				m_linkId = sharedBus->subscribe(TopicId::Topic_Hartbeat, MAKE_CALLBACK(onPulseReceived));
			}
		}
		~HealthMonitor()
		{
			std::cout << "destroying health telemetry: ";
		}

	private:
		void onPulseReceived(const Message& message)
		{
			std::cout << "Pulse log: "
				<< Topic::getStringView(message.topicId) << " : "
				<< message.payload << '\n';
		}
	};

	class Display : public Node
	{
	public:
		Display(std::weak_ptr<Bus> wpBus) : Node(wpBus)
		{
			auto sharedBus = m_wpBus.lock();
			if (sharedBus)
			{
				m_linkId = sharedBus->subscribe(TopicId::Topic_CameraFrame, MAKE_CALLBACK(onCameraFrameReceived));
			}
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

	class CollisionTracker : public Node
	{
	public:
		CollisionTracker(std::weak_ptr<Bus> wpBus) : Node(wpBus)
		{
			auto sharedBus = m_wpBus.lock();
			if (sharedBus)
			{
				m_linkId = sharedBus->subscribe(TopicId::Topic_SensorData, MAKE_CALLBACK(onSensorDataReceived) );
			}
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