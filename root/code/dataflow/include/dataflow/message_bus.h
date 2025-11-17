#ifndef DATAFLOWMESSAGEBUS_H
#define DATAFLOWMESSAGEBUS_H

#include "dataflow/message.h"
#include "dataflow/topic.h"
#include "dataflow/types.h"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <array>

#define MAKE_CALLBACK(x) [this](const Dataflow::Message& m) { x(m); }

namespace Dataflow
{
	class Bus
	{
	public:
		typedef std::function<void(const Message&)> OnMessageCallback;

		Bus();
		~Bus() = default;
		int								 subscribe(TopicId, OnMessageCallback);
		std::unordered_map<TopicId, int> subscribeAll(OnMessageCallback);
		void							 unsubscribe(int);
		void							 publish(const Message&);
		size_t							 getAllPublisherCount() const;
		size_t							 getAllSubscriberCount() const;
		size_t							 getSubscriberCount(TopicId) const;
		size_t							 getPublisherCount(TopicId) const;

		void							 registerPublisher(TopicId);
		void							 unregisterPublisher(TopicId);

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

		// bookkeeping for telemetry
		std::array<std::atomic<uint32_t>, Topic::getTopicCount()> m_pubCountByTopic{}; 
		std::atomic<size_t> m_publishersCount;

		mutable std::mutex m_mutex;
		int m_linksCreatedCount;

		void removeSubFromTopic(TopicId topicId, size_t index);
	};
}

#endif // DATAFLOWMESSAGEBUS_H