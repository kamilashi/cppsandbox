#ifndef NNOBSERVEREMITTER_H
#define NNOBSERVEREMITTER_H

#include <format>
#include <thread>

#include "publisher.h"
#include "node.h"

namespace NNObserver
{
	class PerceptionNode : public Node
	{
	public:
		PerceptionNode(std::string_view id) : 
			Node(id),
			m_cameraPub( TopicId::Topic_CameraFrame),
			m_sensorPub( TopicId::Topic_SensorData),
			m_heartbeatPub( TopicId::Topic_Heartbeat ),
			m_hbSeq(0),
			m_pulseIntervalMs(1000.0f)
		{
			startPulseThread(); //#todo need to handle publish tries before the bus becomes available - queue messages or delay pulse start?
		}

		~PerceptionNode()
		{
			stopPulseThread();
		};

		void registerMessageBus(std::weak_ptr<Bus> wpBus) override
		{
			Node::registerMessageBus(wpBus);
			m_cameraPub.registerPublisher(wpBus);
			m_sensorPub.registerPublisher(wpBus);
			m_heartbeatPub.registerPublisher(wpBus);
		}

		void createFrameData() 
		{
			m_cameraPub.tryPublish(m_nodeName, "Dummy frame data");
		}

		void createSensorData()
		{
			m_sensorPub.tryPublish(m_nodeName, "Dummy collision data");
		}

	private:
		Publisher m_cameraPub;
		Publisher m_sensorPub;
		Publisher m_heartbeatPub;

		//#wip: might turn pulsing into reusable functionality

		int m_hbSeq;
		float m_pulseIntervalMs;
		std::jthread m_pulseThread;

		void pulse()
		{
			m_heartbeatPub.tryPublish(m_nodeName, std::format("seq: {}", m_hbSeq));
			m_hbSeq++;
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

		void stopPulseThread()
		{
			m_pulseThread.request_stop();
		}
	};
}

#endif