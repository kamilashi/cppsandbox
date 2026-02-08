#include "build_config.h"
#include "dataflow/message_bus.h"
#include "dataflow/emitter_components.h"
#include "dataflow/receiver_components.h"
#include "dataflow/topic_inspector.h"
#include "dataflow/node_examples.h"
#include "dataflow/node.h"

#include "malloc_tracker.h"
#include "console_frame_printer.h"
#include "stdexio/event_loop.h"

#include <iostream>
#include <thread>
#include <functional>
#include <memory>

#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>
#include <exec/async_scope.hpp>

namespace Dataflow
{
	void printTopicStats(MultiLineConsoleVisualizer* pVisualizer, TopicInspector* pInspector)
	{
		pVisualizer->visualizeTable(pInspector->getHeader(), pInspector->getBody());
	}

	//void triggerFrameData(PerceptionNode node)
	//{
	//	node.createFrameData();
	//}

	void testComponents()
	{
		PerceptionNode perceptionNode("Perception");
		MultiLineConsoleVisualizer visualizer;

		{
			auto spMessageBus = std::make_shared<Bus>();

			perceptionNode.registerMessageBus(spMessageBus);

			TopicInspector topicInspector(spMessageBus, 1000.0f);

			//HealthMonitor healthTracker("Health Monitor");
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

			std::jthread inspectorThread = std::jthread([&](std::stop_token st)
				{
					while (!st.stop_requested())
					{
						printTopicStats(&visualizer, &topicInspector);
						std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(500.0f)); 
					}
				});

			exec::static_thread_pool eventPool{ 1 };
			stdexio::EventLoop eventLoop{ eventPool.get_scheduler() };

			eventLoop.addEventHandler('c', [&perceptionNode] {
				perceptionNode.createFrameData();
			});

			eventLoop.addEventHandler('s', [&perceptionNode] {
				perceptionNode.createSensorData();
			});

			eventLoop.registerStopKey('e');
			
			stdexio::blockUntilExited(eventLoop); // block until user requests io exit
		}
	}

	void runTest()
	{
		testComponents();
	}
}

int main(int argc, char* argv[])
{
	//MallocTracker::printMemoryAllocationMetrics();
	Dataflow::runTest();
	//MallocTracker::printMemoryAllocationMetrics();
}