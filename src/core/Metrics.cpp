#include "core/Metrics.h"
#include "core/Platform.h"

#include <chrono>
#include <string_view>

namespace adventure
{
	namespace
	{
		constexpr float kEma = 0.1f;               // smoothing factor for display values
		constexpr double kSysSampleInterval = 0.5; // seconds between OS mem/cpu samples
		constexpr double kMaxWindow = 1.0;         // seconds for the frame-time max window

		double steadyNowSeconds()
		{
			using clock = std::chrono::steady_clock;
			static const auto t0 = clock::now();
			return std::chrono::duration<double>(clock::now() - t0).count();
		}
	} // namespace

	Metrics::Metrics()
	    : m_clock(&steadyNowSeconds)
	{
	}

	Metrics& Metrics::instance()
	{
		static Metrics m;
		return m;
	}

	void Metrics::setClock(std::function<double()> nowSeconds)
	{
		m_clock = std::move(nowSeconds);
	}

	Metrics::Section* Metrics::find(const char* name)
	{
		for (auto& s : m_sections)
			if (s.name == name || (s.name && name && std::string_view(s.name) == name))
				return &s;
		return nullptr;
	}

	void Metrics::beginFrame()
	{
		m_frameStart = m_clock();
		for (auto& s : m_sections)
			s.accum = 0.0;
	}

	void Metrics::begin(const char* name)
	{
		Section* s = find(name);
		if (!s)
		{
			m_sections.push_back(Section{name, 0.0, 0.0, 0.0f});
			s = &m_sections.back();
		}
		s->startT = m_clock();
	}

	void Metrics::end(const char* name)
	{
		Section* s = find(name);
		if (s)
			s->accum += (m_clock() - s->startT);
	}

	void Metrics::endFrame()
	{
		const double now = m_clock();

		m_frameMsRaw = (float)((now - m_frameStart) * 1000.0);
		m_frameMs = m_frameMs + kEma * (m_frameMsRaw - m_frameMs);

		// Rolling worst-case frame time (spike detector).
		if (m_frameMsRaw > m_frameMsMax)
			m_frameMsMax = m_frameMsRaw;
		if (now - m_windowStart >= kMaxWindow)
		{
			m_frameMsMaxShown = m_frameMsMax;
			m_frameMsMax = 0.0f;
			m_windowStart = now;
		}

		// Rebuild the display snapshot.
		m_view.clear();
		m_view.reserve(m_sections.size());
		for (auto& s : m_sections)
		{
			const float raw = (float)(s.accum * 1000.0);
			s.smoothMs = s.smoothMs + kEma * (raw - s.smoothMs);
			m_view.push_back(SectionView{s.name, s.smoothMs, raw});
		}

		// Sample OS-level metrics infrequently (they are relatively expensive).
		if (m_lastSysSample < 0.0 || now - m_lastSysSample >= kSysSampleInterval)
		{
			m_lastSysSample = now;
			m_rss = platform::workingSetBytes();
			m_cpuPct = platform::processCpuPercent();
		}
	}

	float Metrics::fps() const
	{
		return m_frameMs > 0.0001f ? 1000.0f / m_frameMs : 0.0f;
	}
} // namespace adventure
