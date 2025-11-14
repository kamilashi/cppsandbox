#include "dataflow/message_bus.h"

namespace Dataflow
{
	Bus::Bus() : 
		m_linksCreatedCount(0), 
		m_subsByTopic(static_cast<size_t>(TopicId::Topic_Count)),
		m_publishersCount(0),
		m_pubCountByTopic(0)
	{
		constexpr auto topicCount = Topic::getTopicCount();
		for (size_t i = 0; i < topicCount; ++i) 
		{
			m_pubCountByTopic[i].store(0, std::memory_order_relaxed);
		}
	}

	int Bus::subscribe(TopicId topicId, OnMessageCallback callback)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		m_linksCreatedCount++;

		const int linkId = m_linksCreatedCount;

		auto& subsPerTopic = m_subsByTopic[topicId]; // create a vector at this key if not already and get the reference
		size_t idByTopic = subsPerTopic.size();
		subsPerTopic.emplace_back(linkId);

		m_allSubs.emplace(linkId, SubData{ topicId, idByTopic, callback });
		return linkId;
	}

	std::unordered_map<TopicId, int> Bus::subscribeAll(OnMessageCallback callback)
	{
		std::unordered_map<TopicId, int> map;
		map.reserve(Topic::getTopicCount());

		for (auto it = Topic::begin(); it < Topic::end(); it++)
		{
			TopicId topic = *it;
			int id = subscribe(topic, callback);
			map.emplace(topic, id);
		}

		return map;
	}

	void Bus::unsubscribe(int linkId)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_allSubs.find(linkId) != m_allSubs.end())
		{
			const SubData& subData = m_allSubs.at(linkId);
			removeSubFromTopic(subData.topicId, subData.indexByTopic);
			m_allSubs.erase(linkId);
		}
	}

	void Bus::publish(const Message& message) 
	{
		std::vector<OnMessageCallback> callbacks;

		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_subsByTopic.find(message.topicId) != m_subsByTopic.end())
			{
				auto& subsPerTopic = m_subsByTopic.at(message.topicId);
				callbacks.reserve(subsPerTopic.size());

				for (size_t subIndx : subsPerTopic)
				{
					callbacks.emplace_back(m_allSubs.at(subIndx).callback);
				}
			}
		}

		// call client code after releasing the mutex
		for (const OnMessageCallback& callback : callbacks)
		{
			callback(message);
		}
	}

	size_t Bus::getAllPublisherCount() const 
	{
		return m_publishersCount.load(std::memory_order_acquire);
	}

	size_t Bus::getAllSubscriberCount() const 
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_allSubs.size();
	}

	size_t Bus::getSubscriberCount(TopicId topicId) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_subsByTopic.find(topicId) != m_subsByTopic.end())
		{
			return m_subsByTopic.at(topicId).size();
		}
		
		return 0;
	}

	size_t Bus::getPublisherCount(TopicId topicId) const
	{
		return m_pubCountByTopic[(size_t) topicId].load(std::memory_order_acquire);
	}

	void Bus::registerPublisher(TopicId topic)
	{
		m_pubCountByTopic[(size_t)topic].fetch_add(1u, std::memory_order_relaxed);
		m_publishersCount.fetch_add(1u, std::memory_order_relaxed);
	}

	void Bus::unregisterPublisher(TopicId topic) 
	{
		auto prev = m_publishersCount.fetch_sub(1, std::memory_order_relaxed);
		auto prevAll = m_pubCountByTopic[(size_t)topic].fetch_sub(1, std::memory_order_relaxed);

		if (prevAll == 0)
		{
			m_publishersCount.store(0, std::memory_order_relaxed);
		}

		if (prev == 0) 
		{
			m_pubCountByTopic[(size_t)topic].store(0, std::memory_order_relaxed);
		}
	}

	void Bus::removeSubFromTopic(TopicId topicId, size_t topicIndex) // o(1) complexity, order does not matter
	{
		auto& subsPerTopic = m_subsByTopic.at(topicId);

		const bool isLast = topicIndex == subsPerTopic.size() - 1;
		if (!isLast)
		{
			subsPerTopic[topicIndex] = subsPerTopic.back();
		}

		subsPerTopic.pop_back();

		if (!isLast && !subsPerTopic.empty())
		{
			auto& sub = m_allSubs.at(subsPerTopic[topicIndex]);
			sub.indexByTopic = topicIndex;
		}
	}
}