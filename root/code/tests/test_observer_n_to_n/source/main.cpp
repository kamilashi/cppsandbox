#include "message_bus.h"
#include "emitter.h"
#include "receiver.h"
#include <cstdio>
#include <thread>
#include <memory>

#define PROFILER

namespace NNObserver
{
	static std::atomic<bool> sIsStopProgramRequested{ false };
	static std::atomic<bool> sIsCameraTriggerRequested{ false };
	static std::atomic<bool> sIsSensorTriggerRequested{ false };

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
				sIsCameraTriggerRequested.store(true, std::memory_order_release);
				break;
			case 's':
				sIsSensorTriggerRequested.store(true, std::memory_order_release);
				break;
			default:
				break;
			}
		};
	}

	void runTest()
	{
		CameraEmitter camera(1000.0f, "Camera");
		SensorEmitter sensor(1000.0f, "Sensor");

		{
			auto spMessageBus = std::make_shared<Bus>();
			sensor.registerMessageBus(spMessageBus);
			camera.registerMessageBus(spMessageBus);

			sensor.startPulseThread();
			camera.startPulseThread();

			HealthTelemetry healthTracker(spMessageBus);
			CollisionTracker collisionTracker(spMessageBus);
			Display display(spMessageBus);

			std::cout << "Press:\n\n"
				<< "c to publish a camera message \n"
				<< "s to publish a sensor message \n"
				<< "p to pulse both \n"
				<< "Enter to submit \n\n"
				<< "e to exit \n\n";


			std::jthread inputThread = std::jthread([](std::stop_token st)
			{
				while (!st.stop_requested())
				{
					readInput();
				}
			});

			while (!sIsStopProgramRequested.load(std::memory_order_acquire))
			{
				if (sIsCameraTriggerRequested.exchange(false, std::memory_order_acq_rel))
				{
					camera.createFrameData();
				}

				if (sIsSensorTriggerRequested.exchange(false, std::memory_order_acq_rel))
				{
					sensor.createSensorData();
				}
			};
		}

		camera.createFrameData();
		__debugbreak();
	}
}


int main(int argc, char* argv[])
{
	NNObserver::runTest();
}