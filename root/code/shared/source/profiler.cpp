#include "profiler.h"

namespace Profiler
{
	static ScopeProfileTree profileTree;

	int  ScopeTimer::sFrameNumber = 0;
	bool ScopeTimer::sIsPaused = false;

	ScopeTimer::ScopeTimer(int id, const char* name) : m_id(id), m_name(name)
	{
		if (sIsPaused)
		{
			return;
		}
		
		m_startTimePoint = std::chrono::high_resolution_clock::now();

		profileTree.onNodeOpen(m_id);
	};

	ScopeTimer::~ScopeTimer()
	{
		if (sIsPaused)
		{
			return;
		}

		auto endTimePoint = std::chrono::high_resolution_clock::now();

		double start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimePoint)
			.time_since_epoch()
			.count();
		start *= 0.001f;

		double end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint)
			.time_since_epoch()
			.count();
		end *= 0.001f;

		double elapsedTime = end - start;

		profileTree.onNodeClose(m_name, start, elapsedTime);
	};

	void onStartFrame()
	{
		if (!ScopeTimer::sIsPaused)
		{
			profileTree.reset();
		}

		ScopeTimer::sFrameNumber++;
	}

/*
	void onEndFrame()
	{
		FrameProfileData frame{};
		getFrame(&frame);

		printf("\r%s%s\x1b[K\r", frame.header, frame.body);
		fflush(stdout);
	}*/

	void onEndFrame(FrameDataVisualizer* pVisualizer)
	{
		FrameProfileData frame{};
		getFrame(&frame);

		pVisualizer->visualizeFrameData(frame);
		
		//printf("\r%s%s\x1b[K\r", frame.header, frame.body);
		//fflush(stdout);
	}

	void endProfiler()
	{
		profileTree.reset();
	}

	void formatProfileRowIndented(char* pBuffer, int bufferSize, const ScopeProfileData& data, const ScopeStats& stats, int depth)
	{
		char indent[64];
		int labelStart = depth * 2;
		int indentLen = labelStart + 3;

		indentLen = indentLen < 60 ? indentLen : 60;
		memset(indent, ' ', labelStart);
		memset(indent + labelStart, '_', indentLen- labelStart);
		indent[labelStart] = '|';
		indent[indentLen] = '\0';

		double average = stats.getMovingAverage();

		size_t len = strlen(pBuffer);
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

		const ScopeStats& statsIt = profileTree.getOrCreateStats(node.id);

		formatProfileRowIndented(pBuffer, bufferSize, node.data, statsIt, depth);

		for (size_t child = node.firstChildIdx; child != ~0u; child = profileTree.getNode(child).nextSiblingIdx)
		{
			writeNode(pBuffer, bufferSize, child, depth + 1);
		}
	}

	void getFrame(FrameProfileData* pFrameData)
	{
		memset(pFrameData->header, 0, FrameProfileData::headerSize);
		memset(pFrameData->body, 0, FrameProfileData::bodySize);

		snprintf(pFrameData->header, FrameProfileData::headerSize, "Frame %08llu | Duration - real (ms) | - max (ms) | - min (ms) | - avg in %u fr (ms) | Scope \n", static_cast<unsigned long long>(ScopeTimer::sFrameNumber), ScopeStats::movingAverageWindowSize);

		if (profileTree.getNodesCount() > 1)
		{
			writeNode(pFrameData->body, FrameProfileData::bodySize, profileTree.getRootIdx(), 0);
		}
	}
}