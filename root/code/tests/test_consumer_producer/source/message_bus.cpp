#include "message_bus.h"

namespace ConsumerProducer
{
    MessageType Message::typeCount_ = 0;

    MessageBus::MessageBus()
    {
    }

    MessageBus::~MessageBus()
    {
        queue_.clear();
    }

    void MessageBus::commitAll()
    {
        queue_.commit();
    }

    void MessageBus::clearAll()
    {
        queue_.clear();
    }

    void MessageBus::clearInactiveMessages()
    {
        queue_.clearAllInactiveMessages();
    }

    Message* MessageBus::getNextMessage()
    {
        return queue_.getOldestActiveMessageAndDeactivate();
    }

    std::size_t MessageBus::count() const
    {
        return queue_.count();
    }

    bool MessageBus::empty() const
    {
        return queue_.count() == 0;
    }
};

