#pragma once
#include <cstddef>
#include <string>
#include <vector>

// Reporting for a headless profiling run. Single responsibility: turn collected frame-time samples +
// a Metrics snapshot into statistics and a CSV file. computeFrameStats is pure (no IO) so it is
// unit-testable; writeProfileCsv is the thin file-writing wrapper.
namespace adventure
{
	class Metrics;

	struct FrameStats
	{
		float       avgMs = 0.0f;
		float       minMs = 0.0f;
		float       p50Ms = 0.0f;
		float       p95Ms = 0.0f;
		float       maxMs = 0.0f;
		std::size_t count = 0;
	};

	// Percentiles use nearest-rank on a sorted copy. Empty input -> all zeros.
	FrameStats computeFrameStats(std::vector<float> samples);

	// Writes a one-run CSV: frame stats, per-section ms, RSS, Lua bytes. Returns false on IO failure.
	bool writeProfileCsv(const std::string& path, const Metrics& metrics,
		const std::vector<float>& frameSamples);
}
