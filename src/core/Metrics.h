#pragma once
#include <cstddef>
#include <functional>
#include <vector>

// Per-frame perf data: timing + memory. No raylib (headless-testable); clock injectable. Drawn by MetricsOverlay.
namespace adventure
{
	class Metrics
	{
	public:
		struct SectionView
		{
			const char* name;
			float ms;    // smoothed (for display)
			float msRaw; // this frame (for asserts)
		};

		Metrics();

		// Convenience global instance used by game code (tests construct their own).
		static Metrics& instance();

		// Test seam: supply a monotonic clock returning seconds. Defaults to steady_clock.
		void setClock(std::function<double()> nowSeconds);

		void beginFrame();
		void endFrame();

		void begin(const char* section);
		void end(const char* section);

		void setLuaBytes(std::size_t bytes) { m_luaBytes = bytes; }

		// RAII section timer.
		struct Scope
		{
			Metrics& m;
			const char* n;
			Scope(Metrics& mm, const char* name)
			    : m(mm), n(name) { m.begin(n); }
			~Scope() { m.end(n); }
		};

		// Snapshot accessors (valid after endFrame).
		float fps() const;
		float frameMs() const { return m_frameMs; }            // smoothed
		float frameMsRaw() const { return m_frameMsRaw; }      // last frame
		float frameMsMax() const { return m_frameMsMaxShown; } // worst in last ~1s window
		float cpuPercent() const { return m_cpuPct; }
		std::size_t rssBytes() const { return m_rss; }
		std::size_t luaBytes() const { return m_luaBytes; }
		const std::vector<SectionView>& sections() const { return m_view; }

	private:
		struct Section
		{
			const char* name;
			double startT;
			double accum;
			float smoothMs;
		};
		Section* find(const char* name);

		std::function<double()> m_clock;
		std::vector<Section> m_sections;
		std::vector<SectionView> m_view;

		double m_frameStart = 0.0;
		float m_frameMsRaw = 0.0f;
		float m_frameMs = 0.0f;
		float m_frameMsMax = 0.0f;      // accumulating max in current window
		float m_frameMsMaxShown = 0.0f; // last completed window's max
		double m_windowStart = 0.0;

		std::size_t m_rss = 0;
		std::size_t m_luaBytes = 0;
		float m_cpuPct = 0.0f;
		double m_lastSysSample = -1.0;
	};
} // namespace adventure
