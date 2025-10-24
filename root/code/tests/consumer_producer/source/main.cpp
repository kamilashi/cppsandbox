#include "message.h"
#include "message_bus.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ConsumerProducer
{
	void processDamageMessage(int* pHealth, const DamageMessage& message)
	{
		const uint damageAmount = message.getDamageAmount();

		if (*pHealth > 0)
		{
			*pHealth -= damageAmount;
			printf("damaged for %u HP, current health: %d \n", damageAmount, *pHealth);
		}

		*pHealth = std::max(*pHealth, 0);
	}

	void processHealMessge(int* pHealth, const HealMessage& message)
	{
		const uint healAmount = message.getHealAmount();

		if (*pHealth > 0)
		{
			*pHealth += healAmount;
			printf("healed for %u HP, current health: %d \n", healAmount, *pHealth);
		}
	}

	void processMessage(int* pHealth, Message* pMessage)
	{
		const MessageType messageType = pMessage->getType();
		if (messageType == DamageMessage::type())
		{
			DamageMessage* pDamageMessage = static_cast<DamageMessage*>(pMessage);
			processDamageMessage(pHealth, *pDamageMessage);
		}
		else if (messageType == HealMessage::type())
		{
			HealMessage* pHealMessage = static_cast<HealMessage*>(pMessage);
			processHealMessge(pHealth, *pHealMessage);
		}
	}

	static std::mutex mtx;
	static std::condition_variable cv;
	static uint frame = 0;
	static uint semaphoreCount = 0;
	static uint maxFrameCount = 10;

	static MessageBus messageBus;

	template <typename MType>
	void process_inputs(MessageBus& messageBus, uint payload)
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [&messageBus] {return (messageBus.empty());  });

			if (frame >= maxFrameCount)
			{
				break;
			}

			messageBus.commitAll();

			MType* pMessage = new MType(payload);
			messageBus.send(pMessage);
			printf("sent type %d message: %u\n", MType::type(), payload);

			messageBus.commitAll();

			++semaphoreCount;
			cv.notify_one();
		}
	}

	void run_simulation(MessageBus& messageBus)
	{
		static int health = 10;

		while (frame < maxFrameCount)
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [] {return semaphoreCount > 0; });

			printf("\nstart of sim. cycle # %u - health: %d \n", frame + 1, health);

			while (!messageBus.empty())
			{
				Message* pMessage = messageBus.getNextMessage();

				if (pMessage != nullptr)
				{
					processMessage(&health, pMessage);
				}
			}

			printf("\n");

			messageBus.clearInactiveMessages();

			++frame;

			--semaphoreCount;
			cv.notify_all();
		}
	}

	void runTest()
	{
		std::thread produce1(process_inputs<DamageMessage>, std::ref(messageBus), 3u);
		std::thread produce2(process_inputs<HealMessage>, std::ref(messageBus), 6u);
		std::thread consume(run_simulation, std::ref(messageBus));

		produce1.join();
		produce2.join();
		consume.join();

		messageBus.clearAll();

		printf("\nRemaining message instances: %u ", Message::instanceCount); // manual check for the memory leaks

		printf("\nPress Enter to exit...");
		while (getchar() != '\n');
	}
}

int Message::instanceCount = 0;

int main(int argc, char* argv[])
{
	ConsumerProducer::runTest();

	return 0;
}
