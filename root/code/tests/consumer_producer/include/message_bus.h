
#ifndef MESSAGEBUS_H
#define MESSAGEBUS_H

#include <assert.h>
#include "message_queue.h"

namespace ConsumerProducer
{
    /// A generic message handler that takes messages via send() and allows iteration over
    /// all committed (see commitAll()) messages.
    class MessageBus
    {
    public:
        typedef std::size_t size_type;

        MessageBus();
        ~MessageBus();

        /// Send the given message.
        ///
        /// @note Note that it will only be activated when commitAll() is called.
	    template <typename MType>
	    void send(MType* message)
	    {
		    queue_.enqueue(message);
	    }

        /// Activates all previously sent messages.
        void commitAll();

        /// Empties all message queues.
        void clearAll();
    
        void clearInactiveMessages();
    
        Message* getNextMessage();

        size_type count() const;

        bool empty() const;
    
    private:
        MessageQueue queue_;
    };
};

#endif
