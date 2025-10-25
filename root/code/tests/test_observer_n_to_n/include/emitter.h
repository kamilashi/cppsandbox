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
		MessageEmitter() : m_pBus(nullptr), m_seq(0), m_pulseIntervalMs(-1), m_id("") {}
		MessageEmitter(float pulseInterval, std::string_view id) : m_pBus(nullptr), m_seq(0), m_pulseIntervalMs(pulseInterval), m_id(id) {}
		~MessageEmitter() = default;

		void registerMessageBus(Bus* pBus)
		{
			m_pBus = pBus;
		}

		void pulse()
		{
			if (!m_pBus)
			{
				return;
			}

			Message message(TopicId::Topic_Hartbeat, std::format("emitter: {}, seq: {}", m_id, m_seq));
			m_pBus->publish(message);
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
		Bus* m_pBus;
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
			if (!m_pBus)
			{
				return;
			}

			Message message(TopicId::Topic_CameraFrame, "cameraFrame: frame");
			m_pBus->publish(message);
		}
	};

	class SensorEmitter : public MessageEmitter
	{
	public:
		SensorEmitter(float pulseInterval, std::string_view id) : MessageEmitter(pulseInterval, id) {}
		~SensorEmitter() = default;

		void createSensorData()
		{
			if (!m_pBus)
			{
				return;
			}

			Message message(TopicId::Topic_SensorData, "sensorData: data");
			m_pBus->publish(message);
		}
	};
}

#endif