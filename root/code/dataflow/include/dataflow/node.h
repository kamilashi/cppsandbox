#ifndef DATAFLOWNODE_H
#define DATAFLOWNODE_H

#include <vector>
#include <thread>
#include <memory>

#include "dataflow/node_input.h"
#include "dataflow/node_output.h"

namespace Dataflow
{
	class BaseNode
	{
	public:
		BaseNode(float intervalM) : m_intervalMs(intervalM)
		{}

		virtual ~BaseNode() 
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

			m_runThread = std::jthread([this](std::stop_token st) {
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

	template <class T>
	concept Process =
		std::default_initializable<T> &&
		requires (T p, std::vector<Input>&inputs, std::vector<Output>&outputs)
	{
		p.fire(inputs, outputs);
	};

	template<Process P>
	class Node
	{
	public:
		Node(float intervalM, 
			std::vector<Input::InitList>&& inputs, 
			std::vector<Output::InitList>&& outputs,
			std::weak_ptr<Bus> wpBus) 
			:
			m_intervalMs(intervalM),
			m_process{}
		{
			m_inputs.reserve(inputs.size());

			for (const Input::InitList& list : inputs)
			{
				m_inputs.emplace_back(list);
			}

			m_outputs.reserve(outputs.size());

			for (const Output::InitList& list : outputs)
			{
				m_outputs.emplace_back(list);
			}

			connectAndStart(wpBus);
		}

		~Node() 
		{
			stopRunThread();
		};

	private:

		void fire()
		{
			m_process.fire(m_inputs, m_outputs);
		}

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

			m_runThread = std::jthread([this](std::stop_token st) {
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
		P m_process;
	};
}

#endif // DATAFLOWNODE_H