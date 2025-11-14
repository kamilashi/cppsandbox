#ifndef DATAFLOWRECEIVER_H
#define DATAFLOWRECEIVER_H

#include "dataflow/subscriber.h"
#include "dataflow/component.h"

namespace Dataflow
{
	class HealthMonitor : public Component
	{
	public:
		HealthMonitor(std::string_view id) : 
			Component(id),
			m_heartbeatSub(TopicId::Topic_Heartbeat)
		{}

		~HealthMonitor()
		{}

		void registerMessageBus(std::weak_ptr<Bus> wpBus) override
		{
			Component::registerMessageBus(wpBus);
			m_heartbeatSub.subscribe(wpBus, MAKE_CALLBACK(onPulseReceived));
		}

	private:
		Subscriber m_heartbeatSub;

		void onPulseReceived(const Message& message)
		{
			std::cout << "Pulse log: "
				<< Topic::getStringView(message.topicId) << " : "
				<< message.payload << '\n';
		}
	};

	class Display : public Component
	{
	public:
		Display(std::string_view id) :
			Component(id),
			m_cameraSub(TopicId::Topic_CameraFrame)
		{}

		~Display()
		{}

		void registerMessageBus(std::weak_ptr<Bus> wpBus) override
		{
			Component::registerMessageBus(wpBus);
			m_cameraSub.subscribe(wpBus, MAKE_CALLBACK(onCameraFrameReceived));
		}

	private:
		Subscriber m_cameraSub;

		void onCameraFrameReceived(const Message& message)
		{
			std::cout << "Frame: " << message.payload << '\n';
		}
	};

	class CollisionTracker : public Component
	{
	public:
		CollisionTracker(std::string_view id) :
			Component(id),
			m_sensorSub(TopicId::Topic_SensorData)
		{}

		~CollisionTracker()
		{}

		void registerMessageBus(std::weak_ptr<Bus> wpBus) override
		{
			Component::registerMessageBus(wpBus);
			m_sensorSub.subscribe(wpBus, MAKE_CALLBACK(onSensorDataReceived));
		}

	private:
		Subscriber m_sensorSub;

		void onSensorDataReceived(const Message& message)
		{
			std::cout << "Collision: " << message.payload << '\n';
		}
	};
}

#endif // DATAFLOWRECEIVER_H