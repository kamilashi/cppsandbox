#include "build_config.h"
#include "dataflow/message_bus.h"
#include "dataflow/emitter_components.h"
#include "dataflow/receiver_components.h"
#include "dataflow/topic_inspector.h"
#include "dataflow/node_examples.h"

#include "malloc_tracker.h"
#include "console_frame_printer.h"

#include <cstdio>
#include <thread>
#include <memory>

namespace Dataflow
{
	static std::atomic<bool> sIsStopProgramRequested{ false };
	static std::atomic<bool> sInspectorPaused{ false };
	static std::atomic<uint32_t> sCamTriggers{ 0 };
	static std::atomic<uint32_t> sSensTriggers{ 0 };

	void readInput()
	{
		int inputChar;
		while (inputChar = getchar())
		{
			if (inputChar == '\r') 
			{
				continue;
			};

			if (inputChar == 'e')
			{
				sIsStopProgramRequested.store(true, std::memory_order_release);
				return;
			};

			switch (inputChar) {
			case 'c':
				sCamTriggers.fetch_add(1, std::memory_order_relaxed);
				break;
			case 's':
				sSensTriggers.fetch_add(1, std::memory_order_relaxed);
				break;
			default:
				break;
			}
		};
	}

	void printTopicStats(MultiLineConsoleVisualizer* pVisualizer, TopicInspector* pInspector)
	{
		if (sIsStopProgramRequested.load(std::memory_order_acquire))
		{
			return;
		}

		pVisualizer->visualizeTable(pInspector->getHeader(), pInspector->getBody());
	}

	void testComponents()
	{
		PerceptionNode perceptionNode("Perception");
		MultiLineConsoleVisualizer visualizer;

		{
			auto spMessageBus = std::make_shared<Bus>();

			perceptionNode.registerMessageBus(spMessageBus);

			TopicInspector topicInspector(spMessageBus, 1000.0f);

			HealthMonitor healthTracker("Health Monitor");
			CollisionTracker collisionTracker("Collision Tracker");
			Display display("Display");

			// commented healthTracker in order for it not to compete with the inspector's output.

			//healthTracker.registerMessageBus(spMessageBus);
			collisionTracker.registerMessageBus(spMessageBus);
			display.registerMessageBus(spMessageBus);


			std::cout << "Press:\n\n"
				<< "c to publish a camera message \n"
				<< "s to publish a sensor message \n"
				<< "Enter to submit \n\n"
				<< "e to exit \n\n";

			std::jthread inputThread = std::jthread([](std::stop_token st)
				{
					while (!st.stop_requested())
					{
						readInput();
					}
				});

			std::jthread inspectorThread = std::jthread([&](std::stop_token st)
				{
					while (!st.stop_requested())
					{
						printTopicStats(&visualizer, &topicInspector);
						std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(500.0f)); 
					}
				});

			while (!sIsStopProgramRequested.load(std::memory_order_acquire))
			{
				// this will ruin the exact order of the messages received, but is okay for now.
				for (uint32_t n = sCamTriggers.exchange(0, std::memory_order_acq_rel); n > 0; --n)
				{
					perceptionNode.createFrameData();
				}

				for (uint32_t n = sSensTriggers.exchange(0, std::memory_order_acq_rel); n > 0; --n)
				{
					perceptionNode.createSensorData();
				}
			};
		}

		perceptionNode.createFrameData();
	}

	void printNodeOperationResult(const Message& resultMessage)
	{
		std::cout << resultMessage.source << " : " << resultMessage.payload << "\n\n";
	}

	struct Diff
	{
		void fire(std::vector<Input>& ins, std::vector<Output>& outs)
		{
			Message op1 = ins[0].consume();
			Message op2 = ins[0].consume();

			Message sum = Message(
				TopicId::Topic_Dif,
				"add",
				std::format(" {} - {} ", op1.userData, op2.userData));

			sum.userData = op1.userData - op2.userData;

			outs[0].produce(sum);
		}
	};

	struct MultSumDiff
	{
		void fire(std::vector<Input>& ins, std::vector<Output>& outs)
		{
			Message op1 = ins[0].consume();
			Message op2 = ins[1].consume();

			Message sum = Message(
				TopicId::Topic_MultSumDif,
				"multiply sum and diff",
				std::format("( {} ) * ( {} )", op1.payload, op2.payload));

			sum.userData = op1.userData * op2.userData;

			outs[0].produce(sum);
		}
	};

	struct Result
	{
		void fire(std::vector<Input>& ins, std::vector<Output>& outs)
		{
			Message op1 = ins[0].consume();

			Message res = Message(
				TopicId::Topic_FinalRes,
				"result",
				std::format(" {} = {} ", op1.payload, op1.userData));

			res.userData = op1.userData;

			outs[0].produce(res);
		}
	};

	void testNodes()
	{
		auto spMessageBus = std::make_shared<Bus>();
		NumberGenNode numGen(spMessageBus);
		AddNode add(spMessageBus);
		//MultSumsNode multSums(spMessageBus);
		//ResultNode result(spMessageBus);

		// template nodes
		Node<Diff> multSumsNode(1000.0f,
			{ {TopicId::Topic_Sum, 2} },
			{ {TopicId::Topic_Dif} },
			spMessageBus);

		Node<MultSumDiff> multSumDiffNode(1000.0f,
			{ {TopicId::Topic_Sum}, {TopicId::Topic_Dif} },
			{ {TopicId::Topic_MultSumDif} }, 
			spMessageBus);

		Node<Result> resultNode(1000.0f,
		{ {TopicId::Topic_MultSumDif} },
		{ {TopicId::Topic_FinalRes} },
			spMessageBus);

		Subscriber numGenPrinter(TopicId::Topic_NumberGen);
		Subscriber sumPrinter(TopicId::Topic_Sum);
		Subscriber multPrinter(TopicId::Topic_MultSums);
		Subscriber resultPrinter(TopicId::Topic_FinalRes);

		resultPrinter.subscribe(spMessageBus, printNodeOperationResult);

		while ( getchar() != '\n')
		{};
	}

	void runTest()
	{
		//testComponents();
		testNodes();
	}
}


int main(int argc, char* argv[])
{
	MallocTracker::printMemoryAllocationMetrics();
	Dataflow::runTest();
	MallocTracker::printMemoryAllocationMetrics();
}