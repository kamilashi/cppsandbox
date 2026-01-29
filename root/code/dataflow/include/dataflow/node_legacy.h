#ifndef DATAFLOWNODELEGACY_H
#define DATAFLOWNODELEGACY_H

#include <vector>
#include <thread>
#include <memory>

#include "dataflow/node_input.h"
#include "dataflow/node_output.h"

namespace Dataflow
{
	class BaseNodeLegacy
	{
	public:
		BaseNodeLegacy(float intervalM) : m_intervalMs(intervalM)
		{}

		virtual ~BaseNodeLegacy() 
		{ 
			stopRunThread(); 
		}

	protected:

		void virtual fire() {}

		void run()
		{
			if (isReady())
			{
				fire();
			}
		}

		void connectAndStart(std::weak_ptr<Bus> wpBus)
		{
			for (Input& input : m_inputs)
			{
				input.initialize(wpBus);
			}

			for (Output& output : m_outputs)
			{
				output.initialize(wpBus);
			}

			startRunThread();
		}

		void startRunThread()
		{
			if (m_intervalMs < 0)
			{
				return;
			}

			m_runThread = std::jthread([this](std::stop_token st) 
			{
				while (!st.stop_requested())
				{
					run();
					std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(m_intervalMs));
				}
			});
		}

		void stopRunThread()
		{
			m_runThread.request_stop();
		}

		bool isReady() const
		{
			if (m_inputs.empty())
			{
				return true;
			}

			for (const Input& input : m_inputs)
			{
				if (!input.isReady())
				{
					return false;
				}
			}

			return true;
		}

		std::vector<Input> m_inputs;
		std::vector<Output> m_outputs;

		float m_intervalMs;
		std::jthread m_runThread;
	};
}

#endif // DATAFLOWNODE_H