#pragma once
#include <cstddef>

// OS-specific process metrics. Single responsibility: talk to the platform, nothing else.
// No raylib here (so it never clashes with <windows.h>); no formatting, no drawing.
namespace adventure::platform
{
	// Resident memory of this process, in bytes (Windows working set). 0 if unavailable.
	std::size_t workingSetBytes();

	// This process's CPU utilisation as a percentage of one core's worth of time, measured
	// over the interval since the previous call (0..100*cores). First call seeds and returns 0.
	float processCpuPercent();
}
