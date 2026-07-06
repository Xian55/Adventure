#pragma once
#include <cstddef>
#include <string>
#include <vector>

// Profiling-run reporting: frame-time samples -> stats + CSV. computeFrameStats is pure (testable).
namespace adventure
{
	class Metrics;

	struct FrameStats
	{
		float avgMs = 0.0f;
		float minMs = 0.0f;
		float p50Ms = 0.0f;
		float p95Ms = 0.0f;
		float maxMs = 0.0f;
		std::size_t count = 0;
	};

	// Percentiles use nearest-rank on a sorted copy. Empty input -> all zeros.
	FrameStats computeFrameStats(std::vector<float> samples);

	// Writes a one-run CSV: frame stats, per-section ms, RSS, Lua bytes. Returns false on IO failure.
	bool writeProfileCsv(const std::string& path, const Metrics& metrics, const std::vector<float>& frameSamples);
} // namespace adventure
