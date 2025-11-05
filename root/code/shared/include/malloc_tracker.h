#ifndef MALLOCTRACKER_H
#define MALLOCTRACKER_H

#include "buildconfig.h"
#include <memory>
#include <cstdlib> 
#include <new> 

namespace MallocTracker
{
	struct MallocMetrics
	{
		size_t bytesAllocated;
		size_t bytesFreed;

		MallocMetrics() :
			bytesAllocated(0),
			bytesFreed(0)
		{}
	};
}

#ifdef TRACKMALLOC
	
extern MallocTracker::MallocMetrics mallocMetrics;
MallocTracker::MallocMetrics mallocMetrics;

void* operator new(size_t bytes)
{
	mallocMetrics.bytesAllocated += bytes;

	return malloc(bytes);
}

void operator delete(void* address, size_t bytes)
{
	mallocMetrics.bytesFreed += bytes;

	free(address);
}

namespace MallocTracker
{
	static MallocMetrics& getMetrics()
	{
		return mallocMetrics;
	}
}
	
#endif // TRACKMALLOC


#endif // MALLOCTRACKER_H