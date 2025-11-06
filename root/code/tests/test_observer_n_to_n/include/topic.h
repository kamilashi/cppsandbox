#ifndef NNOBSERVERTOPIC_H
#define NNOBSERVERTOPIC_H

#include <string>
#include <vector>
#include <array>
	
namespace NNObserver
{
	enum class TopicId : size_t
	{
		Topic_Heartbeat,
		Topic_SensorData,
		Topic_CameraFrame,

		Topic_NumberGen,
		Topic_Sum,
		Topic_Dif,
		Topic_MultSums,
		Topic_MultSumDif,
		Topic_FinalRes,

		Topic_Count
	};

	namespace Topic
	{
		//static const std::vector<const char*> sTopicMap = {"heartbeat", "sensor/data", "camera/frame"};
		inline constexpr std::array<const char*, static_cast<size_t>(TopicId::Topic_Count)> sTopicMap
		{
			"heartbeat", 
			"sensor/data", 
			"camera/frame"
		};

		inline static const char* getString(TopicId id)
		{
			if (id < TopicId::Topic_Count)
			{
				return sTopicMap[static_cast<size_t>(id)];
			}

			return "Unknown";
		}

		inline static std::string_view getStringView(TopicId id)
		{
			return getString(id);
		}

		inline static constexpr size_t getTopicCount()
		{
			return static_cast<size_t>(TopicId::Topic_Count);
		}

		class iterator
		{
		public:
			iterator(TopicId start) : m_counter(static_cast<size_t>(start))
			{}

			iterator(size_t start) : m_counter(start) 
			{}

			iterator& operator ++()
			{
				m_counter++;
				return *this;
			}

			iterator operator ++(int)
			{
				iterator it = iterator(m_counter);
				m_counter++;
				return it;
			}

			bool operator==(const iterator& other) const
			{
				return m_counter == other.m_counter;
			}

			bool operator!=(const iterator& other) const
			{
				return m_counter != other.m_counter;
			}

			bool operator < (const iterator& other) const
			{
				return m_counter < other.m_counter;
			}

			bool operator > (const iterator& other) const
			{
				return m_counter > other.m_counter;
			}

			bool operator <= (const iterator& other) const
			{
				return m_counter <= other.m_counter;
			}

			bool operator >= (const iterator& other) const
			{
				return m_counter >= other.m_counter;
			}

			TopicId operator*() const
			{
				return static_cast<TopicId>(m_counter);
			}
		
		private:
			size_t m_counter;
		};

		static iterator begin()
		{
			return iterator{ 0u };
		}

		static iterator end()
		{
			return iterator{ TopicId::Topic_Count };
		}
	}
}

#endif // NNOBSERVERTOPIC_H