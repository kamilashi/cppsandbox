#include <cstdio>
#include <cstring>

#include "profiler.h"

// this code was NOT written by me. It's just a quick implementation for the visualizer interface

class MultiLineConsoleVisualizer : public Profiler::FrameDataVisualizer
{
public:
	MultiLineConsoleVisualizer() : m_prevLines(0) { /*enableAnsiOnce();*/ }

	void visualizeFrameData(const Profiler::FrameProfileData& frameData) override
	{
		char buf[1024];
		int n = std::snprintf(buf, sizeof(buf), "%s%s", frameData.header, frameData.body);
		if (n < 0) return;
		buf[sizeof(buf) - 1] = '\0';

		if (m_prevLines > 0) {
			if (m_prevLines > 1) std::printf("\x1b[%zuA", m_prevLines - 1); 
			std::printf("\r");
			for (size_t i = 0; i < m_prevLines; ++i) 
			{
				std::printf("\x1b[K");               
				if (i + 1 < m_prevLines) std::printf("\n");
			}
			if (m_prevLines > 1) std::printf("\x1b[%zuA", m_prevLines - 1); 
			std::printf("\r");
		}

		const char* p = buf;
		while (*p) 
		{
			const char* nl = std::strchr(p, '\n');
			if (nl) 
			{
				std::printf("%.*s\x1b[K\n", int(nl - p), p);
				p = nl + 1;
			}
			else {
				std::printf("%s\x1b[K", p); 
				break;
			}
		}
		std::fflush(stdout);

		m_prevLines = countLines(buf);
	}

private:
	size_t m_prevLines;

	static size_t countLines(const char* s)
	{
		if (!s || !*s) return 1;
		size_t lines = 1;
		for (const char* p = s; *p; ++p) if (*p == '\n') ++lines;
		return lines;
	}
/*

	static void enableAnsiOnce()
	{
#ifdef _WIN32
		static bool done = false;
		if (done) return;
		void* h = GetStdHandle(STD_OUTPUT_HANDLE);
		if (h && h != INVALID_HANDLE_VALUE) {
			DWORD mode = 0;
			if (GetConsoleMode(h, &mode)) {
				mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
				SetConsoleMode(h, mode);
			}
		}
		done = true;
#endif
	}*/
};