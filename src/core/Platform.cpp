#include "core/Platform.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>

namespace adventure::platform
{
	std::size_t workingSetBytes()
	{
		PROCESS_MEMORY_COUNTERS pmc{};
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
			return static_cast<std::size_t>(pmc.WorkingSetSize);
		return 0;
	}

	float processCpuPercent()
	{
		static ULONGLONG lastWall = 0; // 100ns ticks
		static ULONGLONG lastCpu = 0;
		static int cores = 0;

		if (cores == 0)
		{
			SYSTEM_INFO si{};
			GetSystemInfo(&si);
			cores = (int)si.dwNumberOfProcessors;
			if (cores < 1)
				cores = 1;
		}

		FILETIME ftCreate, ftExit, ftKernel, ftUser;
		if (!GetProcessTimes(GetCurrentProcess(), &ftCreate, &ftExit, &ftKernel, &ftUser))
			return 0.0f;

		ULARGE_INTEGER k, u;
		k.LowPart = ftKernel.dwLowDateTime;
		k.HighPart = ftKernel.dwHighDateTime;
		u.LowPart = ftUser.dwLowDateTime;
		u.HighPart = ftUser.dwHighDateTime;
		const ULONGLONG cpu = k.QuadPart + u.QuadPart;

		FILETIME ftNow;
		GetSystemTimeAsFileTime(&ftNow);
		ULARGE_INTEGER w;
		w.LowPart = ftNow.dwLowDateTime;
		w.HighPart = ftNow.dwHighDateTime;
		const ULONGLONG wall = w.QuadPart;

		float pct = 0.0f;
		if (lastWall != 0 && wall > lastWall)
		{
			const double dCpu = (double)(cpu - lastCpu);
			const double dWall = (double)(wall - lastWall);
			pct = (float)(dCpu / dWall / cores * 100.0);
		}
		lastWall = wall;
		lastCpu = cpu;
		return pct;
	}
} // namespace adventure::platform

#else // non-Windows stub (implement per-platform later)

namespace adventure::platform
{
	std::size_t workingSetBytes()
	{
		return 0;
	}
	float processCpuPercent()
	{
		return 0.0f;
	}
} // namespace adventure::platform

#endif
