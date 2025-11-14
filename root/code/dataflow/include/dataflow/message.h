#ifndef DATAFLOWMESSAGE_H
#define DATAFLOWMESSAGE_H

#include "dataflow/topic.h"
#include <string>
	
namespace Dataflow
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

#endif // DATAFLOWMESSAGE_H