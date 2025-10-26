#ifndef PROFILER_H
#define PROFILER_H

#include <vector>
#include <unordered_map>
#include <chrono>
#include <algorithm> 
#include <deque>
#include <iostream>

#include "buildconfig.h"
#include "helpers.h"

namespace Profiler
{
	struct FrameProfileData
	{
		static const size_t headerSize = 128;
		static const size_t bodySize = 256;
		char header[headerSize];
		char body[bodySize];
	};

	class FrameDataVisualizer
	{
	public:
		virtual void visualizeFrameData(const FrameProfileData& frameData) = 0;
	};

	class ScopeTimer
	{
	public:
		static int sFrameNumber;
		static bool sIsPaused;

		ScopeTimer(int id, const char* name);

		~ScopeTimer();

	private:
		int m_id;
		const char* m_name;
		std::chrono::time_point<std::chrono::steady_clock> m_startTimePoint;
	};

	struct ScopeProfileData
	{
	public:
		const char* name;
		double startTime;
		double elapsedTime;

		ScopeProfileData() : 
			name (""),
			startTime(0.0),
			elapsedTime(0.0)
		{};

		ScopeProfileData(const char* n, double startT, double elapsedT) : 
			name(n),
			startTime(startT), 
			elapsedTime(elapsedT) {};
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
		{};

		ScopeStats(double minTime, double maxTime) :
			minElapsedTime(minTime),
			maxElapsedTime(maxTime), 
			m_movingAveHeadIdx(0),
			m_movingAveWindow{}
		{};

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
		{};

		ScopeProfileNode(int id, size_t parent) : 
			id(id), 
			parentIdx(parent),
			firstChildIdx(~0u),
			lastChildIdx(~0u),
			nextSiblingIdx(~0u)
		{};

		void setData(const char* n, double startT, double elapsedT)
		{
			data.name = n;
			data.startTime = startT;
			data.elapsedTime = elapsedT;
		};
	};
	 
	struct ScopeProfileTree
	{
	public:
		ScopeProfileTree() : 
			m_nodes(m_sMaxNodesCount),
			m_openNodes(m_sMaxNodesCount),
			m_stats(m_sMaxNodesCount) {};

		~ScopeProfileTree() {};

		void reset() 
		{
			m_nodes.clear();
			m_openNodes.clear();

			pushNode(~0u, ~0u); // root
			m_openNodes.emplace_back(0);
		};

		void onNodeOpen(int id)
		{
			const size_t parentIdx = m_openNodes.back();
			const size_t thisIdx = m_nodes.size();

#ifdef _DEBUG
			if (thisIdx == m_sMaxNodesCount)
			{
				std::cerr << "Increase the default capacity of the profiler's node storage!" << "\n";
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
		};
		
		void onNodeClose(const char* n, double startT, double elapsedT)
		{
			if (m_openNodes.size() <= 1) return;
			const size_t lastOpenIdx = m_openNodes.back();
			m_nodes[lastOpenIdx].setData(n, startT, elapsedT);
			m_openNodes.pop_back();

			storeStats(&m_nodes[lastOpenIdx].data, m_nodes[lastOpenIdx].id);
		};

		const ScopeProfileNode& getNode(int idx) const
		{
			return m_nodes[idx];
		};

		const ScopeStats& getStats(int idKey) const
		{
			return m_stats.at(idKey);
		};

		const ScopeStats& getOrCreateStats(int idKey)
		{
			return m_stats[idKey];
		};

		bool hasStatsFor(int idKey) const
		{
			return m_stats.find(idKey) != m_stats.end();
		};

		size_t getNodesCount() const 
		{
			return m_nodes.size();
		};

		size_t getRootIdx() const
		{
			return m_sStartIdx;
		};

	private:
		static const size_t m_sStartIdx = 1;
		static const size_t m_sMaxNodesCount = 64;
		std::vector<ScopeProfileNode> m_nodes;
		std::vector<size_t> m_openNodes;
		std::unordered_map<int, ScopeStats> m_stats;

		void pushNode(size_t parent, int id)
		{
			m_nodes.emplace_back(id, parent);
		};

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
		};
	};

	void onStartFrame();
	void onEndFrame(FrameDataVisualizer* pVisualizer);
	void endProfiler();

	void getFrame(FrameProfileData* pFrameData);
}

#endif // PROFILER_H

#ifndef PROFILER_MACROS_DEFINED
#define PROFILER_MACROS_DEFINED

#if defined (PROFILE)							
	#define PROFILE_SCOPE(nameLiteral)		Profiler::ScopeTimer timer(FILELINE_ID, nameLiteral)
	#define PROFILER_START_FRAME() 			Profiler::onStartFrame()
	#define PROFILER_END_FRAME(x) 			Profiler::onEndFrame(x)
	#define PROFILER_END() 					Profiler::endProfiler()							
#else
	#define PROFILER_INIT()					do {} while (0)
	#define PROFILE_SCOPE(nameLiteral)		do {} while (0)
	#define PROFILER_END_FRAME()			do {} while (0)
	#define PROFILER_END()					do {} while (0)	
#endif // PROFILE

#endif // PROFILER_MACROS_DEFINED
