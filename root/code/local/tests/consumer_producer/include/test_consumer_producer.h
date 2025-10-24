
#ifndef CONSUMERPRODUCER_H
#define CONSUMERPRODUCER_H

#include "message_bus.h"
#include "types.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ConsumerProducer
{
	void run();
}

#endif //CONSUMERPRODUCER_H
