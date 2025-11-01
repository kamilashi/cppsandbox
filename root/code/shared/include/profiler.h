#ifndef PROFILER_H
#define PROFILER_H

#include <chrono>

#include "buildconfig.h"
#include "helpers.h"

namespace Profiler
{
	struct FrameProfileData
	{
		static const size_t headerSize = 128;
		static const size_t bodySize = 1024;
		char header[headerSize];
		char body[bodySize];
	};

	class FrameDataVisualizer
	{
	public:
		virtual void visualizeTable(const FrameProfileData& frameData) = 0;
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

	void onStartFrame();
	void onEndFrame(FrameDataVisualizer* pVisualizer);
	void endProfiler();
}

#endif // PROFILER_H

#ifndef PROFILER_MACROS_DEFINED
#define PROFILER_MACROS_DEFINED

#if defined (PROFILE)							
#define PROFILE_SCOPE(nameLiteral)			Profiler::ScopeTimer timer(FILELINE_ID, nameLiteral)
#define PROFILER_START_FRAME() 				Profiler::onStartFrame()
#define PROFILER_END_FRAME(visualizer) 		Profiler::onEndFrame(visualizer)
#define PROFILER_END() 						Profiler::endProfiler()							
#else
#define PROFILER_INIT()						do {} while (0)
#define PROFILE_SCOPE(nameLiteral)			do {} while (0)
#define PROFILER_END_FRAME(visualizer)		do {} while (0)
#define PROFILER_END()						do {} while (0)	
#endif // PROFILE

#endif // PROFILER_MACROS_DEFINED
