#ifndef NNOBSERTOPICINSPECTOR_H
#define NNOBSERTOPICINSPECTOR_H

#include "node.h"
#include "topic.h"
#include "movingaverage.h"

#include <unordered_map>
#include <chrono>
#include <mutex>
#include <atomic>
#include <memory>

namespace NNObserver
{
	class IntervalLoad
	{
	public:
		double intervalSeconds;
		double load;

		IntervalLoad() :
			intervalSeconds(0.0),
			load(0)
		{}

		explicit IntervalLoad(double defaultValue) :
			intervalSeconds(defaultValue), 
			load(defaultValue)
		{}

		explicit IntervalLoad(double seconds, double load) :
			intervalSeconds(seconds),
			load(load)
		{}

		explicit IntervalLoad(int defaultValue) : IntervalLoad(static_cast<double>(defaultValue)) {}

		IntervalLoad& operator/= (size_t scalar)
		{
			double scalarD = static_cast<double>(scalar);
			intervalSeconds /= scalarD;
			load /= scalarD;
			return *this;
		}

		IntervalLoad& operator+= (const IntervalLoad& other)
		{
			intervalSeconds += other.intervalSeconds;
			load += other.load;
			return *this;
		}
	};

	struct TableCell
	{
		const char* name;
		size_t width;

		TableCell() : name(""), width(0)
		{ }

		TableCell(const char* name) : name(name), width(strlen(name))
		{ }

		TableCell(const char* name, size_t minWidth) : name(name),  width(std::max(strlen(name), minWidth))
		{ }

		void updateWidth(size_t minWidth)
		{
			width = std::max(width, minWidth);
		}
	};

	struct TopicStats
	{
		static const size_t movingAverageWindowSize = 5;
		double hz;
		double bps;
		double lastSeenAgeMs;
		std::chrono::time_point<std::chrono::steady_clock> lastSeenTimePoint;
		MovingAverage<IntervalLoad> movingAveSec;
		int connectionId;

		TopicStats(int id) : 
			hz(0.0),
			bps(0.0),
			lastSeenAgeMs(-1.0),
			lastSeenTimePoint{},
			movingAveSec(movingAverageWindowSize),
			connectionId(id)
		{}

		bool hasLastSeenMessage() const
		{
			return lastSeenAgeMs >= 0;
		}
	};

	class TopicInspector : public Node
	{
	public:
		TopicInspector(std::weak_ptr<Bus> wpBus, float tickIntervalMs) : Node("Topic Inspector")
		{ 
			Node::registerMessageBus(wpBus);
			start(tickIntervalMs); 
		};

		~TopicInspector() 
		{ 
			shutDown(); 
		};

		const char* getHeader() 
		{
			return m_header;
		}

		const char* getBody()
		{
			size_t skipped = 0;
			size_t offset = 0;
			for (auto& kv : m_stats)
			{
				size_t idx = static_cast<size_t> (kv.first);

				if (m_mutexes[idx].try_lock())
				{
					auto topicName = Topic::getString(kv.first);
					TopicStats& stats = kv.second;

					const size_t cellMaxWidth = 10;
					char m_cellBuffers[6 * cellMaxWidth];
					memset(m_cellBuffers, 0, sizeof(m_cellBuffers));

					size_t subsCount = 0;

					auto sharedBus = m_wpBus.lock();
					if (sharedBus)
					{
						subsCount = sharedBus->getSubscriberCount(kv.first);
					}

					m_headerCells[0].updateWidth(strlen(topicName)),
					m_headerCells[1].updateWidth((size_t)snprintf(m_cellBuffers + cellMaxWidth,		cellMaxWidth, "%.3f", stats.hz)),
					m_headerCells[2].updateWidth((size_t)snprintf(m_cellBuffers + cellMaxWidth * 2, cellMaxWidth, "%.3f", stats.bps)),
					m_headerCells[3].updateWidth((size_t)snprintf(m_cellBuffers + cellMaxWidth * 3, cellMaxWidth, "%.3f", stats.lastSeenAgeMs));
					m_headerCells[4].updateWidth((size_t)snprintf(m_cellBuffers + cellMaxWidth * 4, cellMaxWidth, "%zu", subsCount)),
					m_headerCells[5].updateWidth((size_t)snprintf(m_cellBuffers + cellMaxWidth * 5, cellMaxWidth, "%d", -1 ));

					char row[m_headerSize];
					snprintf(row, m_headerSize, " %-*s | %-*s | %-*s | %-*s | %-*s | %-*s \n",
						m_headerCells[0].width, topicName,
						m_headerCells[1].width, &m_cellBuffers[cellMaxWidth],
						m_headerCells[2].width, &m_cellBuffers[cellMaxWidth * 2],
						m_headerCells[3].width, &m_cellBuffers[cellMaxWidth * 3],
						m_headerCells[4].width, &m_cellBuffers[cellMaxWidth * 4],
						m_headerCells[5].width, &m_cellBuffers[cellMaxWidth * 5]);

					size_t len = strlen(row);
					row[len] = ' ';				// remove null termination

					memcpy(m_body + offset, row, len);

					m_rowLengths[idx] = len;
					m_mutexes[idx].unlock();
				}

				offset += m_rowLengths[idx];
			}

			m_body[offset] = 0;

			// update header cells' widths
			snprintf(m_header, m_headerSize, " %-*s | %-*s | %-*s | %-*s | %-*s | %-*s \n",
				m_headerCells[0].width, m_headerCells[0].name,
				m_headerCells[1].width, m_headerCells[1].name,
				m_headerCells[2].width, m_headerCells[2].name,
				m_headerCells[3].width, m_headerCells[3].name,
				m_headerCells[4].width, m_headerCells[4].name,
				m_headerCells[5].width, m_headerCells[5].name);

			return m_body;
		}

	private:
		void start(float tickIntervalMs)
		{
			m_tickIntervalMs = tickIntervalMs;

			auto sharedBus = m_wpBus.lock();
			auto connections = std::move(sharedBus->subscribeAll(MAKE_CALLBACK(onMessagePublished)));

			m_stats.reserve(connections.size());
			m_rowLengths.resize(connections.size());
			m_mutexes = std::vector<std::mutex>(connections.size()); // note great, but only happens once on creation.

			for (auto& kv : connections)
			{
				std::lock_guard<std::mutex> lock(m_mutexes[static_cast<size_t> (kv.first)]);

				m_stats.emplace(kv.first, TopicStats{ kv.second });
			}

			m_tickThread = std::jthread([this](std::stop_token st) 
			{
				float intervalDurationMs = 0.0f;
				while (!st.stop_requested())
				{
					auto start = std::chrono::steady_clock::now();
					std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(intervalDurationMs));
					tick(intervalDurationMs);

					auto end = std::chrono::steady_clock::now();
					double intervalSec = std::chrono::duration<double>(end - start).count();

					intervalDurationMs = intervalSec * 1000.0;
				}
			});

			memset(m_header, 0, m_headerSize);
			memset(m_body, 0, m_bodySize);
		}

		void onMessagePublished(const Message& message)
		{
			auto topicIdx = static_cast<size_t>(message.topicId);
			std::lock_guard<std::mutex> lock(m_mutexes[topicIdx]);

			TopicStats& stats = m_stats.at(message.topicId);

			auto timePoint = std::chrono::steady_clock::now();
			double intervalSec = 0.0;

			if (stats.hasLastSeenMessage()) 
			{
				intervalSec = std::chrono::duration<double>(timePoint - stats.lastSeenTimePoint).count();
			}

			const size_t byteCount = message.payload.size();
			//double kbLoad = static_cast<double> (byteCount) * 1024.0;
			double bLoad = static_cast<double> (byteCount);

			stats.lastSeenAgeMs = 0.0;
			stats.lastSeenTimePoint = timePoint;
			stats.movingAveSec.pushItem(IntervalLoad{ intervalSec, bLoad });

			const auto avg = stats.movingAveSec.getMovingAverage();

			if (avg.intervalSeconds > 0.0)
			{
				stats.hz = 1.0 / avg.intervalSeconds;
				stats.bps = avg.load / avg.intervalSeconds;
			}
			else
			{
				stats.hz = 0.0;
				stats.bps = 0.0;
			}
		}

		void shutDown()
		{
			auto sharedBus = m_wpBus.lock();

			for (auto& kv : m_stats)
			{
				sharedBus->unsubscribe(kv.second.connectionId);
			}

			if (m_tickThread.joinable()) 
			{
				m_tickThread.request_stop();   
				m_tickThread.join();           
			}

			m_stats.clear();
		}

		void tick(double elapsedTime)
		{
			for (auto& kv : m_stats)
			{
				std::lock_guard<std::mutex> lock(m_mutexes[static_cast<size_t> (kv.first)]);

				TopicStats& stats = kv.second;

				if (stats.hasLastSeenMessage())
				{
					stats.lastSeenAgeMs += elapsedTime;
				}
			}
		}

		float m_tickIntervalMs;
		std::unordered_map<TopicId, TopicStats> m_stats;
		std::vector<std::mutex> m_mutexes;
		std::vector<size_t> m_rowLengths;
		TableCell m_headerCells[6] = {{"Topic"}, {"hz"}, {"bps"}, {"last age (ms)"}, {"subs"}, {"pubs"}};
		std::jthread m_tickThread;

		static const size_t m_headerSize = 128;
		static const size_t m_bodySize = 1024;
		char m_header[m_headerSize];
		char m_body[m_bodySize];
	};
}

#endif // NNOBSERTOPICINSPECTOR_H