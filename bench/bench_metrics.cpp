#include "Bench.h"
#include "core/Metrics.h"

// The metrics instrumentation runs every frame around every measured section, so its own overhead
// must stay negligible. These bench the RAII scope + frame accounting with an injected clock (so we
// measure our code, not the OS clock).
static double g_t = 0.0;

// Full frame accounting: beginFrame + several sections + endFrame.
ADV_BENCH(metrics_frame_with_sections, 200000, 600.0)
{
	static adventure::Metrics m = [] {
		adventure::Metrics mm;
		mm.setClock([] { return g_t; });
		return mm;
	}();

	m.beginFrame();
	{ adventure::Metrics::Scope s(m, "a"); g_t += 0.001; }
	{ adventure::Metrics::Scope s(m, "b"); g_t += 0.001; }
	{ adventure::Metrics::Scope s(m, "c"); g_t += 0.001; }
	g_t += 0.001;
	m.endFrame();
}

// Isolated cost of a single begin/end section pair.
ADV_BENCH(metrics_scope_pair, 1000000, 300.0)
{
	static adventure::Metrics m = [] {
		adventure::Metrics mm;
		mm.setClock([] { return g_t; });
		return mm;
	}();

	m.begin("x");
	g_t += 0.0001;
	m.end("x");
}
