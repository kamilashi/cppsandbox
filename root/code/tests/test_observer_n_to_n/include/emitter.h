#ifndef NNOBSERVEREMITTER_H
#define NNOBSERVEREMITTER_H

#include <format>
#include <thread>
#include "message_bus.h"

namespace NNObserver
{
	class MessageEmitter
	{
	public:
		MessageEmitter() : m_wpBus(), m_seq(0), m_pulseIntervalMs(-1), m_id("") {}
		MessageEmitter(float pulseInterval, std::string_view id) : m_wpBus(), m_seq(0), m_pulseIntervalMs(pulseInterval), m_id(id) {}
		~MessageEmitter() 
		{ 
			unregisterMessageBus(); 
		}

		void registerMessageBus(std::weak_ptr<Bus> wpBus)
		{
			m_wpBus = std::move(wpBus);

			auto sharedBus = m_wpBus.lock();
			if (!sharedBus)
			{
				sharedBus->registerPublisher(); // wip
			}
		}

		void unregisterMessageBus()
		{
			auto sharedBus = m_wpBus.lock();
			if (!sharedBus)
			{
				sharedBus->unregisterPublisher(); // wip
			}
		}

		void pulse()
		{
			auto sharedBus = m_wpBus.lock();
			if (!sharedBus)
			{
				return;
			}

			Message message(TopicId::Topic_Hartbeat, m_id, std::format("emitter: {}, seq: {}", m_id, m_seq));
			sharedBus->publish(message);
			m_seq++;
		}

		void startPulseThread()
		{
			if (m_pulseIntervalMs < 0)
			{
				return;
			}

			m_pulseThread = std::jthread([this](std::stop_token st) {
				while (!st.stop_requested()) 
				{ 
					pulse();
					std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(m_pulseIntervalMs));
				}
			});
		}

	protected:
		//Bus* m_pBus;
		std::weak_ptr<Bus> m_wpBus;
		int m_seq;
		float m_pulseIntervalMs;
		std::jthread m_pulseThread;
		std::string_view m_id;
	};


	class CameraEmitter : public MessageEmitter
	{
	public:
		CameraEmitter(float pulseInterval, std::string_view id) : MessageEmitter(pulseInterval, id) {}
		~CameraEmitter() = default;

		void createFrameData() 
		{
			auto sharedBus = m_wpBus.lock();
			if (!sharedBus)
			{
				return;
			}

			Message message(TopicId::Topic_CameraFrame, m_id, "cameraFrame: frame");
			sharedBus->publish(message);
		}
	};

	class SensorEmitter : public MessageEmitter
	{
	public:
		SensorEmitter(float pulseInterval, std::string_view id) : MessageEmitter(pulseInterval, id) {}
		~SensorEmitter() = default;

		void createSensorData()
		{
			auto sharedBus = m_wpBus.lock();
			if (!sharedBus)
			{
				return;
			}

			Message message(TopicId::Topic_SensorData, m_id, "sensorData: data");
			sharedBus->publish(message);
		}
	};
}

#endif