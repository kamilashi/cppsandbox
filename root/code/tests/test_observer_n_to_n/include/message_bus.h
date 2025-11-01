#ifndef NNOBSERVERMESSAGEBUS_H
#define NNOBSERVERMESSAGEBUS_H

#include "message.h"
#include "topic.h"
#include <unordered_map>
#include <unordered_set>
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
		size_t							 getAllPublisherCount() const;
		size_t							 getAllSubscriberCount() const;
		size_t							 getSubscriberCount(TopicId topicId) const;

		void							 registerPublisher();
		void							 unregisterPublisher();

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

		struct PubData // wip
		{
			size_t indexByTopic;
			std::string_view name;

			PubData(size_t indexByTopic, std::string_view name) :
				indexByTopic(indexByTopic),
				name(name)
			{ };

			~PubData() = default;
		};

		std::unordered_map<TopicId, std::vector<int>> m_subsByTopic;
		std::unordered_map<TopicId, std::vector<int>> m_pubsByTopic; // wip

		std::unordered_map<int, SubData> m_allSubs;
		std::unordered_map<int, PubData> m_allPubs; // wip


		mutable std::mutex m_mutex;
		int m_linksCreatedCount;
		size_t m_publishersCount;

		void removeSubFromTopic(TopicId topicId, size_t index);
	};
}

#endif // NNOBSERVERMESSAGEBUS_H