#include <doctest/doctest.h>
#include "core/Metrics.h"

#include <string_view>

using namespace adventure;

// A Metrics instance is independently constructable (not forced through the singleton), and its
// clock is injectable — so timing is fully deterministic under test.
TEST_CASE("section timing accumulates against an injected clock")
{
	double t = 0.0;
	Metrics m;
	m.setClock([&t] { return t; });

	m.beginFrame();                 // frame start @ t=0
	m.begin("work"); t = 0.010; m.end("work");   // 10 ms
	m.begin("draw"); t = 0.015; m.end("draw");   // 5 ms
	t = 0.016;
	m.endFrame();                   // frame = 16 ms

	CHECK(m.frameMsRaw() == doctest::Approx(16.0));

	float work = -1.0f, draw = -1.0f;
	for (const auto& s : m.sections())
	{
		if (std::string_view(s.name) == "work") work = s.msRaw;
		if (std::string_view(s.name) == "draw") draw = s.msRaw;
	}
	CHECK(work == doctest::Approx(10.0));
	CHECK(draw == doctest::Approx(5.0));
}

TEST_CASE("the same section accumulates across multiple begin/end in one frame")
{
	double t = 0.0;
	Metrics m;
	m.setClock([&t] { return t; });

	m.beginFrame();
	m.begin("ai"); t = 0.002; m.end("ai");   // 2 ms
	m.begin("ai"); t = 0.005; m.end("ai");   // +3 ms
	t = 0.006;
	m.endFrame();

	float ai = -1.0f;
	for (const auto& s : m.sections())
		if (std::string_view(s.name) == "ai") ai = s.msRaw;
	CHECK(ai == doctest::Approx(5.0));
}

TEST_CASE("per-frame accumulators reset each frame")
{
	double t = 0.0;
	Metrics m;
	m.setClock([&t] { return t; });

	m.beginFrame();
	m.begin("x"); t = 0.010; m.end("x");
	t = 0.011; m.endFrame();

	t = 1.000; m.beginFrame();
	m.begin("x"); t = 1.001; m.end("x");   // 1 ms this frame
	t = 1.002; m.endFrame();

	float x = -1.0f;
	for (const auto& s : m.sections())
		if (std::string_view(s.name) == "x") x = s.msRaw;
	CHECK(x == doctest::Approx(1.0));
}

TEST_CASE("fps derives from smoothed frame time")
{
	double t = 0.0;
	Metrics m;
	m.setClock([&t] { return t; });

	// Feed steady 10 ms frames; smoothed value converges toward 10 ms -> ~100 fps.
	for (int i = 0; i < 400; ++i)
	{
		m.beginFrame();
		t += 0.010;
		m.endFrame();
	}
	CHECK(m.frameMs() == doctest::Approx(10.0).epsilon(0.05));
	CHECK(m.fps() == doctest::Approx(100.0).epsilon(0.05));
}
