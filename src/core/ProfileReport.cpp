#include "core/ProfileReport.h"
#include "core/Metrics.h"

#include <algorithm>
#include <fstream>
#include <numeric>

namespace adventure
{
	namespace
	{
		// Nearest-rank percentile on an already-sorted vector (p in [0,1]).
		float percentile(const std::vector<float>& sorted, float p)
		{
			if (sorted.empty()) return 0.0f;
			std::size_t idx = (std::size_t)(p * (sorted.size() - 1) + 0.5f);
			if (idx >= sorted.size()) idx = sorted.size() - 1;
			return sorted[idx];
		}
	}

	FrameStats computeFrameStats(std::vector<float> samples)
	{
		FrameStats s;
		s.count = samples.size();
		if (samples.empty()) return s;

		std::sort(samples.begin(), samples.end());
		const double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
		s.avgMs = (float)(sum / samples.size());
		s.minMs = samples.front();
		s.maxMs = samples.back();
		s.p50Ms = percentile(samples, 0.50f);
		s.p95Ms = percentile(samples, 0.95f);
		return s;
	}

	bool writeProfileCsv(const std::string& path, const Metrics& metrics,
		const std::vector<float>& frameSamples)
	{
		const FrameStats fs = computeFrameStats(frameSamples);

		std::ofstream f(path);
		if (!f) return false;

		f << "metric,value\n";
		f << "frames," << fs.count << "\n";
		f << "frame_avg_ms," << fs.avgMs << "\n";
		f << "frame_min_ms," << fs.minMs << "\n";
		f << "frame_p50_ms," << fs.p50Ms << "\n";
		f << "frame_p95_ms," << fs.p95Ms << "\n";
		f << "frame_max_ms," << fs.maxMs << "\n";
		f << "fps_avg," << (fs.avgMs > 0.0001f ? 1000.0f / fs.avgMs : 0.0f) << "\n";
		f << "rss_bytes," << metrics.rssBytes() << "\n";
		f << "lua_bytes," << metrics.luaBytes() << "\n";
		for (const auto& sec : metrics.sections())
			f << "section_ms_" << (sec.name ? sec.name : "?") << "," << sec.ms << "\n";

		return true;
	}
}
