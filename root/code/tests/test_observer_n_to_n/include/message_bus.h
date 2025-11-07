#ifndef NNOBSERVERMESSAGEBUS_H
#define NNOBSERVERMESSAGEBUS_H

#include "message.h"
#include "topic.h"
#include "types.h"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <array>

#define MAKE_CALLBACK(x) [this](const NNObserver::Message& m) { x(m); }

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
		void							 publish(const Message&);
		size_t							 getAllPublisherCount() const;
		size_t							 getAllSubscriberCount() const;
		size_t							 getSubscriberCount(TopicId topicId) const;
		size_t							 getPublisherCount(TopicId topicId) const;

		void							 registerPublisher(TopicId topic);
		void							 unregisterPublisher(TopicId topic);

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
		std::array<std::atomic<uint32_t>, Topic::getTopicCount()> m_pubCountByTopic{};     // number of Publisher handles
		std::atomic<size_t> m_publishersCount;

		mutable std::mutex m_mutex;
		int m_linksCreatedCount;

		void removeSubFromTopic(TopicId topicId, size_t index);
	};
}

#endif // NNOBSERVERMESSAGEBUS_H