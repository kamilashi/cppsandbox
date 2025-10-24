
#ifndef MESSAGE_H
#define MESSAGE_H

#include "types.h"

namespace ConsumerProducer
{
    /// Type of message type IDs.
    typedef int MessageType;

    /// Non-templated base class for messages.
    ///
    /// @note Note that specific messages should derive from BaseMessage<T>.
    /// 
    /// @sa BaseMessage
    /// @sa MessageBus::send()
    class Message
    {
    public:
	    Message() { ++instanceCount; type_ = -1;}
	    virtual ~Message() {}
        MessageType getType() {return type_;}

	    static int instanceCount;
    protected:
        static MessageType typeCount_;
	    MessageType type_;
    };
    
    /// Base class for all messages.
    ///
    /// @tparam T Type of the derived message. Use like this: class MyMessage : public BaseMessage<MyMessage> ...
    template <class T>
    class BaseMessage : public Message
    {
    public:
	    BaseMessage()
	    {
		    type_ = T::type();
	    }

        static MessageType type()
        {
            static MessageType id = typeCount_++;
            return id;
	    }
    };

    class DamageMessage : public BaseMessage<DamageMessage>
    {
        uint damageAmount_;

    public:
	    DamageMessage(uint amount)
        { 
            damageAmount_ = amount;
        }

        ~DamageMessage()
        { 
            --instanceCount; 
        }

        uint getDamageAmount() const
        {
            return damageAmount_;
        }
    };

    class HealMessage : public BaseMessage<HealMessage>
    {
        uint healAmount_;

    public:
        HealMessage(uint amount)
        {
            healAmount_ = amount;
        }

	    ~HealMessage()
        {
            --instanceCount; 
        }

        uint getHealAmount() const
	    {
		    return healAmount_;
	    }
    };
};

#endif
