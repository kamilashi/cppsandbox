#ifndef NNOBSERVEREMITTER_H
#define NNOBSERVEREMITTER_H

#include <format>
#include "message_bus.h"

namespace NNObserver
{
	class MessageEmitter
	{
	public:
		MessageEmitter() : m_pBus(nullptr), m_seq(0) {}
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

			Message message(TopicId::Topic_Hartbeat, std::format("seq: {}", m_seq));
			m_pBus->publish(message);
			m_seq++;
		}

	protected:
		Bus* m_pBus;
		int m_seq;
	};


	class CameraEmitter : public MessageEmitter
	{
	public:
		CameraEmitter() = default;
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
		SensorEmitter() = default;
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