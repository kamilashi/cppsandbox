
#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <vector>
#include <memory>
#include <type_traits>

#include "message.h"

namespace ConsumerProducer
{
    /// Interface so different MessageQueue instances can be held in a container.
    class MessageQueueInterface
    {
    public:
        virtual ~MessageQueueInterface() { }
        virtual void commit() = 0;
	    virtual void clear() = 0;
	    virtual void clearAllInactiveMessages() = 0;
        virtual Message* getOldestActiveMessageAndDeactivate() = 0;
	    virtual std::size_t count() const = 0;
    };

    /// A list of messages that can be iterated over.
    class MessageQueue : public MessageQueueInterface
    {
    public:
	    typedef std::size_t size_type;
	    typedef std::vector<Message*> MessageList;

	    MessageQueue() : activeMessageStart_(0), messageCount_(0) {}    
        ~MessageQueue() override
	    {
		    clear(); // delete all allocated messages
	    }
  
        /// Copies the given message into the queue.
        template <typename MType>
        void enqueue(MType* message)
	    {
		    static_assert(std::is_base_of<Message, MType>::value, "passed type must derive from Message");
		    if (messages_.size() < messages_.capacity())
		    {
			    messages_.push_back(message);
		    }
		    else
		    {
			    overflowMessages_.push_back(message);
		    }
        }
    
        /// Makes all pending messages active in addition to those that already were active before.
        void commit() override
        {
		    if (!overflowMessages_.empty())
		    {
			    messages_.insert(messages_.end(), overflowMessages_.begin(), overflowMessages_.end());
			    overflowMessages_.clear();
		    }

		    messageCount_ = messages_.size();
        }

        /// Discards all messages.
        void clear() override
        {
            clearQueue(&messages_);
            clearQueue(&overflowMessages_);

		    messageCount_ = 0;
		    activeMessageStart_ = 0;
        }

        typedef typename MessageList::iterator iterator;
        typedef typename MessageList::const_iterator const_iterator;
    
        iterator begin() { return messages_.begin() + activeMessageStart_; }
        iterator end() { return messages_.begin() + messageCount_; }
        const_iterator begin() const { return messages_.begin() + activeMessageStart_; }
        const_iterator end() const { return messages_.begin() + messageCount_; }

	    /// Retrieves the oldest message while marking it as inactive.
        Message* getOldestActiveMessageAndDeactivate() override
	    {
            Message* pMessage = nullptr;
        
            if (activeMessageStart_ < messageCount_)
            {
                pMessage = messages_[activeMessageStart_];
                ++activeMessageStart_;
            }

            return pMessage;
        }

        void clearAllInactiveMessages() override
	    {
            for (size_t i = 0; i < activeMessageStart_; i++)
            {
			    Message*& pMessage = messages_[i];
			    delete pMessage;
                pMessage = nullptr;
            }

            messages_.erase(messages_.begin(), messages_.begin() + activeMessageStart_);

		    messageCount_ = messages_.size();
		    activeMessageStart_ = 0;
        }

        /// Returns the number of active messages that can be iterated over.
        std::size_t count() const override { return messageCount_ - activeMessageStart_; }

    private:
        /// Holds all messages.
        MessageList messages_;
        /// Possibly holds any pending messages that, if they were appended to messages_,
        /// would have caused it to invalidate its iterators.
        MessageList overflowMessages_;
        /// Offset in messages_ that basically hides all messages at a lower position.
        std::size_t activeMessageStart_;
        /// Specifies how many messages (from the start) in messages_ are active.
        std::size_t messageCount_;


	    void clearQueue(MessageList* pQueue)
	    {
            if (pQueue->size() != 0)
		    {
			    for (Message*& msg : *pQueue)
			    {
				    delete msg;
				    msg = nullptr;
			    }
            }

		    pQueue->clear();
	    }
    };
};

#endif
