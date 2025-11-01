#include "message_bus.h"
#include "emitter.h"
#include "receiver.h"
#include "topic_inspector.h"
#include "console_frame_printer.h"
#include <cstdio>
#include <thread>
#include <memory>

#define PROFILER

namespace NNObserver
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

	void runTest()
	{
		CameraEmitter camera(1000.0f, "Camera");
		SensorEmitter sensor(1000.0f, "Sensor");
		MultiLineConsoleVisualizer visualizer;

		{
			auto spMessageBus = std::make_shared<Bus>();
			sensor.registerMessageBus(spMessageBus);
			camera.registerMessageBus(spMessageBus);

			sensor.startPulseThread();
			camera.startPulseThread();

			TopicInspector topicInspector(spMessageBus, 1000.0f);

			//HealthMonitor healthTracker(spMessageBus);
			//CollisionTracker collisionTracker(spMessageBus);
			//Display display(spMessageBus);

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
					std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(500.0f)); // otherwise the inspector tick thread is starving
				}
			});

			while (!sIsStopProgramRequested.load(std::memory_order_acquire))
			{
				// this will ruin the exact order of the messages received, but is okay for now.
				for (uint32_t n = sCamTriggers.exchange(0, std::memory_order_acq_rel); n > 0; --n) 
				{
					camera.createFrameData();
				}

				for (uint32_t n = sSensTriggers.exchange(0, std::memory_order_acq_rel); n > 0; --n) 
				{
					sensor.createSensorData();
				}
			};
		}

		camera.createFrameData();
	}
}


int main(int argc, char* argv[])
{
	NNObserver::runTest();
}