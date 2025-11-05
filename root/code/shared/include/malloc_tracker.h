#ifndef MALLOCTRACKER_H
#define MALLOCTRACKER_H

#include <memory>
#include <cstdlib> 
#include <new> 
#include <iostream> 

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

#if defined TRACKMALLOC
	
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
	inline static MallocMetrics& getMetrics()
	{
		return mallocMetrics;
	}
}
#endif // TRACKMALLOC

namespace MallocTracker
{
	void printMemoryAllocationMetrics()
	{
		static size_t sampleCount = 1;
#if defined TRACKMALLOC
		auto& metrics = MallocTracker::getMetrics();
		std::cout << "\n\n"
			<< "Memory allocation sample " << sampleCount << ": \n"
			<< "allocated:\n"
			<< metrics.bytesAllocated << "\n\n"
			<< "freed: \n"
			<< metrics.bytesFreed << "\n\n"
			<< "leaked: \n"
			<< metrics.bytesAllocated - metrics.bytesFreed << "\n\n";

		sampleCount++;
#else
		std::cout << "\n" << "memory allocation tracking is off.\n\n";
#endif // 
	}
}


#endif // MALLOCTRACKER_H