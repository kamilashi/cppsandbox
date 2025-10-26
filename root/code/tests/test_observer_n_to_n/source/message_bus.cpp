#include "message_bus.h"

namespace NNObserver
{
	Bus::Bus() : m_linksCreatedCount(0), m_subsByTopic(static_cast<size_t>(TopicId::Topic_Count)) {};

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
	};

	void Bus::unsubscribe(int linkId)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_allSubs.find(linkId) != m_allSubs.end())
		{
			const SubData& subData = m_allSubs.at(linkId);
			removeSubFromTopic(subData.topicId, subData.indexByTopic);
			m_allSubs.erase(linkId);
		}
	};

	void Bus::publish(const Message& message) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_subsByTopic.find(message.topicId) != m_subsByTopic.end())
		{
			auto& subsPerTopic = m_subsByTopic.at(message.topicId);
			for (size_t i = 0; i < subsPerTopic.size(); i++)
			{
				m_allSubs.at(subsPerTopic[i]).callback(message);
			}
		}
	};

	void Bus::removeSubFromTopic(TopicId topicId, size_t topicIndex) // o(1) complexity, order does not matter
	{
		auto& subsPerTopic = m_subsByTopic.at(topicId);
		subsPerTopic[topicIndex] = subsPerTopic.back();
		subsPerTopic.pop_back();

		if (!subsPerTopic.empty())
		{
			auto& sub = m_allSubs.at(subsPerTopic[topicIndex]);
			sub.indexByTopic = topicIndex;
		}
	};
}