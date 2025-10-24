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

	void InitProfiler()
	{
	}

	void OnStartFrame()
	{
		if (!ScopeTimer::sIsPaused)
		{
			profileTree.reset();
		}

		ScopeTimer::sFrameNumber++;
	}

	void OnEndFrame()
	{
		DrawImGui();
	}

	void EndProfiler()
	{
		profileTree.reset();
	}

	std::string FormatProfileRowIndented(const ScopeProfileData& data, const ScopeStats& stats, int depth)
	{
		char buffer[256];
		char indent[64];
		int labelStart = depth * 2;
		int indentLen = labelStart + 3;

		indentLen = indentLen < 60 ? indentLen : 60;
		std::memset(indent, ' ', labelStart);
		std::memset(indent + labelStart, '_', indentLen- labelStart);
		indent[labelStart] = '|';
		indent[indentLen] = '\0';

		double average = stats.getMovingAverage();

		std::snprintf(
			buffer, sizeof(buffer),
			"               | %16.6f     | %10.6f | %10.6f |      %10.6f      | %s%-12s",
			data.elapsedTime,
			stats.maxElapsedTime,
			stats.minElapsedTime,
			average,
			indent,
			data.name 
		);

		return std::string(buffer);
	}

	void PrintNode(size_t idx, int depth)
	{
		if (idx == ~0u) return;

		const ScopeProfileNode& node = profileTree.getNode(idx);

		const ScopeStats& statsIt = profileTree.getOrCreateStats(node.id);

		const std::string line = FormatProfileRowIndented(node.data, statsIt, depth);
		// print line line

		for (size_t child = node.firstChildIdx; child != ~0u; child = profileTree.getNode(child).nextSiblingIdx)
		{
			PrintNode(child, depth + 1);
		}
	}

	void DrawImGui()
	{
		//ImGui::Begin("Profiler");
		//ImGui::Checkbox("Pause", &ScopeTimer::sIsPaused);

		//ImGui::Text("Frame %08llu | Duration - real (ms) | - max (ms) | - min (ms) | - avg in %u fr (ms) | Scope ", static_cast<unsigned long long>(ScopeTimer::sFrameNumber), ScopeStats::movingAverageWindowSize);
		//ImGui::Separator();

		if (profileTree.getNodesCount() > 1)
		{
			PrintNode(profileTree.getRootIdx(), 0);
		}

		//ImGui::End();
	}
}