#include "message_bus.h"
#include "emitter.h"
#include "receiver.h"
#include <cstdio>

namespace NNObserver
{
	void runTest()
	{
		Bus messageBus;
		CameraEmitter camera;
		camera.registerMessageBus(&messageBus);
		Display display(&messageBus);

		{
			SensorEmitter sensor;

			sensor.registerMessageBus(&messageBus);

			HealthTelemetry healthTracker(&messageBus);
			CollisionTracker collisionTracker(&messageBus);

			int inputChar;

			std::cout << "Press:\n\n"
				<< "c to publish a camera message \n"
				<< "s to publish a sensor message \n"
				<< "p to pulse both \n"
				<< "Enter to submit \n\n"
				<< "e to exit \n\n";

			while (inputChar = getchar())
			{
				if (inputChar == '\r') continue;
				if (inputChar == 'e') break;

				switch (inputChar) {
				case 'c':
					camera.createFrameData();
					break;
				case 's':
					sensor.createSensorData();
					break;
				case 'p':
					camera.pulse();
					sensor.pulse();
					break;
				default:
					break;
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