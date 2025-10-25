#ifndef NNOBSERVERMESSAGE_H
#define NNOBSERVERMESSAGE_H

#include "topic.h"
#include <string>
	
namespace NNObserver
{
	struct Message
	{
		TopicId topicId;
		std::string payload;

		Message(TopicId topicId, std::string_view payload) : topicId(topicId), payload(payload) {};
		~Message() = default;
	};
}

#endif // NNOBSERVERMESSAGE_H