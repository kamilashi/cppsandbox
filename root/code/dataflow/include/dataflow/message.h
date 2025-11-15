#ifndef DATAFLOWMESSAGE_H
#define DATAFLOWMESSAGE_H

#include "dataflow/topic.h"
#include <string>
#include <ostream>
	
namespace Dataflow
{
	struct Message
	{
		TopicId topicId;
		float userData;
		std::string source;
		std::string payload;

		Message(TopicId topicId, std::string_view source, std::string_view payload) : 
			topicId(topicId),
			userData(0),
			source(source), 
			payload(payload)
		{};

		Message() :
			topicId(TopicId::Topic_Count),
			userData(0),
			source("Unknown"),
			payload("None")
		{};
	};

	inline bool operator==(const Message& lhs, const Message& rhs)
	{
		return lhs.topicId == rhs.topicId
			&& lhs.userData == rhs.userData
			&& lhs.source == rhs.source
			&& lhs.payload == rhs.payload;
	}

	inline bool operator!=(const Message& lhs, const Message& rhs)
	{
		return !(lhs == rhs);
	}

	inline std::ostream& operator<<(std::ostream& os, const Message& msg)
	{
		os  << "Message{"
			<< "topic = " << Topic::getString(msg.topicId) << ", "
			<< "userData = " << msg.userData << ", "
			<< "source = \"" << msg.source << "\", "
			<< "payload = \"" << msg.payload << "\""
			<< "}";

		return os;
	}
}

#endif // DATAFLOWMESSAGE_H