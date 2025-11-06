#ifndef NNOBSERVERMESSAGE_H
#define NNOBSERVERMESSAGE_H

#include "topic.h"
#include <string>
	
namespace NNObserver
{
	struct Message
	{
		TopicId topicId;
		std::string source;
		std::string payload;
		float userData;

		Message(TopicId topicId, std::string_view source, std::string_view payload) : 
			topicId(topicId), 
			source(source), 
			payload(payload), 
			userData(0) 
		{};
	};
}

#endif // NNOBSERVERMESSAGE_H