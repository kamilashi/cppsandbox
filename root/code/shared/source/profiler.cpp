#include "profiler.h"

namespace Profiler
{
	struct ScopeProfileData
	{
	public:
		const char* name;
		double startTime;
		double elapsedTime;

		ScopeProfileData() :
			name(""),
			startTime(0.0),
			elapsedTime(0.0)
		{}

		ScopeProfileData(const char* n, double startT, double elapsedT) :
			name(n),
			startTime(startT),
			elapsedTime(elapsedT)
		{}
	};

	struct ScopeProfileNode
	{
	public:
		ScopeProfileData data;
		int id;

		size_t parentIdx;
		size_t firstChildIdx;
		size_t lastChildIdx;
		size_t nextSiblingIdx;

		ScopeProfileNode() :
			id(-1),
			parentIdx(~0u),
			firstChildIdx(~0u),
			lastChildIdx(~0u),
			nextSiblingIdx(~0u)
		{}

		ScopeProfileNode(int id, size_t parent) :
			id(id),
			parentIdx(parent),
			firstChildIdx(~0u),
			lastChildIdx(~0u),
			nextSiblingIdx(~0u)
		{}

		void setData(const char* n, double startT, double elapsedT)
		{
			data.name = n;
			data.startTime = startT;
			data.elapsedTime = elapsedT;
		}
	};

	struct ScopeStats
	{
	public:
		double minElapsedTime;
		double maxElapsedTime;
		static const size_t movingAverageWindowSize = 5 * 60;

		ScopeStats() :
			minElapsedTime(FLT_MAX),
			maxElapsedTime(0.0f),
			m_movingAveHeadIdx(0),
			m_movingAveWindow{}
		{}

		ScopeStats(double minTime, double maxTime) :
			minElapsedTime(minTime),
			maxElapsedTime(maxTime),
			m_movingAveHeadIdx(0),
			m_movingAveWindow{}
		{}

		void pushElapsedTime(double time)
		{
			m_movingAveWindow[m_movingAveHeadIdx] = time;
			m_movingAveHeadIdx = m_movingAveHeadIdx == movingAverageWindowSize - 1 ? 0 : m_movingAveHeadIdx + 1;
		}

		double getMovingAverage() const
		{
			double ave = 0;
			for (int i = 0; i < movingAverageWindowSize; i++)
			{
				ave += m_movingAveWindow[i];
			}
			ave /= movingAverageWindowSize;
			return ave;
		}

	private:
		double m_movingAveWindow[movingAverageWindowSize];
		size_t m_movingAveHeadIdx;
	};

	struct ScopeProfileTree
	{
	public:
		ScopeProfileTree() :
			m_nodes(0),
			m_openNodes(0),
			m_stats(0)
		{
			m_nodes.reserve(m_sMaxNodesCount);
			m_openNodes.reserve(m_sMaxNodesCount);
			m_stats.reserve(m_sMaxNodesCount);
		}

		~ScopeProfileTree() {}

		void reset()
		{
			m_nodes.clear();
			m_openNodes.clear();

			pushNode(~0u, ~0u); // root
			m_openNodes.emplace_back(0);
		}

		void onNodeOpen(int id)
		{
			const size_t parentIdx = m_openNodes.back();
			const size_t thisIdx = m_nodes.size();

#ifdef _DEBUG
			if (thisIdx == m_sMaxNodesCount)
			{
				printf("Increase the default capacity of the profiler's node storage!\n");
			}
#endif // DEBUG

			pushNode(parentIdx, id);

			ScopeProfileNode& parentNode = m_nodes[parentIdx];
			if (parentNode.firstChildIdx == ~0u) {
				parentNode.firstChildIdx = thisIdx;
				parentNode.lastChildIdx = thisIdx;
			}
			else {
				m_nodes[parentNode.lastChildIdx].nextSiblingIdx = thisIdx;
				parentNode.lastChildIdx = thisIdx;
			}

			m_openNodes.emplace_back(thisIdx);
		}

		void onNodeClose(const char* n, double startT, double elapsedT)
		{
			if (m_openNodes.size() <= 1) return;
			const size_t lastOpenIdx = m_openNodes.back();
			m_nodes[lastOpenIdx].setData(n, startT, elapsedT);
			m_openNodes.pop_back();

			storeStats(&m_nodes[lastOpenIdx].data, m_nodes[lastOpenIdx].id);
		}

		const ScopeProfileNode& getNode(int idx)
		{
			return m_nodes.at(idx);
		}

		const ScopeStats& getOrCreateStats(int idKey)
		{
			return m_stats[idKey];
		}

		bool hasStatsFor(int idKey) const
		{
			return m_stats.find(idKey) != m_stats.end();
		}

		size_t getNodesCount() const
		{
			return m_nodes.size();
		}

		size_t getRootIdx() const
		{
			return m_sStartIdx;
		}

	private:
		static const size_t m_sStartIdx = 1;
		static const size_t m_sMaxNodesCount = 64;
		std::vector<ScopeProfileNode> m_nodes;
		std::vector<size_t> m_openNodes;
		std::unordered_map<int, ScopeStats> m_stats;

		void pushNode(size_t parent, int id)
		{
			m_nodes.emplace_back(id, parent);
		}

		void storeStats(ScopeProfileData* pData, int id)
		{
			auto it = m_stats.find(id);

			if (it != m_stats.end())
			{
				ScopeStats& nodeStats = it->second;
				nodeStats.maxElapsedTime = std::max(nodeStats.maxElapsedTime, pData->elapsedTime);
				nodeStats.minElapsedTime = std::min(nodeStats.minElapsedTime, pData->elapsedTime);
				nodeStats.pushElapsedTime(pData->elapsedTime);
			}
			else
			{
				m_stats.emplace(id, ScopeStats{ pData->elapsedTime, pData->elapsedTime });
			}
		}
	};

	static ScopeProfileTree profileTree;

	int  ScopeTimer::sFrameNumber = 0;
	bool ScopeTimer::sIsPaused = false;

	ScopeTimer::ScopeTimer(int id, const char* name) : m_id(id), m_name(name)
	{
		if (sIsPaused)
		{
			return;
		}
		
		m_startTimePoint = std::chrono::steady_clock::now();

		profileTree.onNodeOpen(m_id);
	};

	ScopeTimer::~ScopeTimer()
	{
		if (sIsPaused)
		{
			return;
		}

		auto endTimePoint = std::chrono::steady_clock::now();

		const auto start_us = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimePoint)
			.time_since_epoch().count();
		const auto end_us = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint)
			.time_since_epoch().count();

		const double start_ms = static_cast<double>(start_us) / 1000.0;
		const double end_ms = static_cast<double>(end_us) / 1000.0;
		const double elapsed = end_ms - start_ms;

		profileTree.onNodeClose(m_name, start_ms, elapsed);
	};

	void formatProfileRowIndented(char* pBuffer, size_t bufferSize, const ScopeProfileData& data, const ScopeStats& stats, int depth)
	{
		const int maxIndent = 60;
		char indent[maxIndent + 2];

		int labelStart = depth * 2;
		if (labelStart > maxIndent) labelStart = maxIndent;

		int indentLen = labelStart + 3;
		if (indentLen > maxIndent) indentLen = maxIndent;

		memset(indent, ' ', labelStart);
		memset(indent + labelStart, '_', indentLen - labelStart);
		indent[labelStart] = '|';
		indent[indentLen] = '\0';

		double average = stats.getMovingAverage();

		size_t len = strlen(pBuffer);

#ifdef _DEBUG
		if (len >= bufferSize)
		{
			printf("Increase the default buffer capacity of the frame profile data! Result is truncated \n");
			return;
		}
#endif //_DEBUG
		snprintf(
			pBuffer + len, bufferSize - len,
			"               | %16.6f     | %10.6f | %10.6f |      %10.6f      | %s%-12s \n",
			data.elapsedTime,
			stats.maxElapsedTime,
			stats.minElapsedTime,
			average,
			indent,
			data.name
		);
	}

	void writeNode(char* pBuffer, int bufferSize, size_t idx, int depth)
	{
		if (idx == ~0u) return;

		const ScopeProfileNode& node = profileTree.getNode(idx);

		const ScopeStats& stats = profileTree.getOrCreateStats(node.id);

		formatProfileRowIndented(pBuffer, bufferSize, node.data, stats, depth);

		for (size_t child = node.firstChildIdx; child != ~0u; child = profileTree.getNode(child).nextSiblingIdx)
		{
			writeNode(pBuffer, bufferSize, child, depth + 1);
		}
	}

	void getFrame(FrameProfileData* pFrameData)
	{
		memset(pFrameData->header, 0, FrameProfileData::headerSize);
		memset(pFrameData->body, 0, FrameProfileData::bodySize);

		snprintf(pFrameData->header, FrameProfileData::headerSize, "Frame %08llu | Duration - real (ms) | - max (ms) | - min (ms) | - avg in %zu fr (ms) | Scope \n", static_cast<unsigned long long>(ScopeTimer::sFrameNumber), ScopeStats::movingAverageWindowSize);

		if (profileTree.getNodesCount() > 1)
		{
			writeNode(pFrameData->body, FrameProfileData::bodySize, profileTree.getRootIdx(), 0);
		}
	}

	void onStartFrame()
	{
		if (!ScopeTimer::sIsPaused)
		{
			profileTree.reset();
		}

		ScopeTimer::sFrameNumber++;
	}

	void onEndFrame(FrameDataVisualizer* pVisualizer)
	{
		FrameProfileData frame{};
		getFrame(&frame);

		pVisualizer->visualizeFrameData(frame);
	}

	void endProfiler()
	{
		profileTree.reset();
	}
}