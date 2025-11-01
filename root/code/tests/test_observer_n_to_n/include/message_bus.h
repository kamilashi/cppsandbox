#ifndef NNOBSERVERMESSAGEBUS_H
#define NNOBSERVERMESSAGEBUS_H

#include "message.h"
#include "topic.h"
#include <unordered_map>
#include <functional>
#include <mutex>

namespace NNObserver
{
	class Bus
	{
	public:
		typedef std::function<void(const Message&)> OnMessageCallback;

		Bus();
		~Bus() = default;
		int								 subscribe(TopicId topicId, OnMessageCallback callback);
		std::unordered_map<TopicId, int> subscribeAll(OnMessageCallback callback);
		void							 unsubscribe(int linkId);
		void							 publish(const Message&) const;

	private:
		struct SubData
		{
			TopicId topicId;
			size_t indexByTopic;
			OnMessageCallback callback;

			SubData(TopicId topicId, size_t indexByTopic, OnMessageCallback callback) :
				topicId(topicId),
				indexByTopic(indexByTopic),
				callback(callback)
			{ };

			~SubData() = default;
		};

		std::unordered_map<TopicId, std::vector<int>> m_subsByTopic;
		std::unordered_map<int, SubData> m_allSubs;
		mutable std::mutex m_mutex;
		int m_linksCreatedCount;

		void removeSubFromTopic(TopicId topicId, size_t index);
	};
}

#endif // NNOBSERVERMESSAGEBUS_H