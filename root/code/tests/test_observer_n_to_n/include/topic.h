#ifndef NNOBSERVERTOPIC_H
#define NNOBSERVERTOPIC_H

#include <string>
#include <vector>
	
namespace NNObserver
{
	enum class TopicId
	{
		Topic_Hartbeat,
		Topic_SensorData,
		Topic_CameraFrame,

		Topic_Count
	};

	namespace Topic
	{
		static const std::vector<std::string_view> sTopicMap = { "heartbeat", "sensor/data", "camera/frame" };

		inline static std::string_view getString(TopicId id)
		{
			if (id < TopicId::Topic_Count)
			{
				return sTopicMap[static_cast<size_t>(id)];
			}
		}
	}
}

#endif // NNOBSERVERTOPIC_H