#include <doctest/doctest.h>
#include "core/ProfileReport.h"

using namespace adventure;

TEST_CASE("computeFrameStats handles an empty sample set")
{
	FrameStats s = computeFrameStats({});
	CHECK(s.count == 0);
	CHECK(s.avgMs == doctest::Approx(0.0));
	CHECK(s.maxMs == doctest::Approx(0.0));
}

TEST_CASE("computeFrameStats computes avg/min/max/percentiles")
{
	// 1..10 ms
	std::vector<float> samples = { 5, 1, 9, 3, 7, 2, 8, 4, 10, 6 };
	FrameStats s = computeFrameStats(samples);

	CHECK(s.count == 10);
	CHECK(s.minMs == doctest::Approx(1.0));
	CHECK(s.maxMs == doctest::Approx(10.0));
	CHECK(s.avgMs == doctest::Approx(5.5));
	// nearest-rank: p95 index = round(0.95*9)=9 -> 10; p50 index = round(0.5*9)=5 -> value 6
	CHECK(s.p95Ms == doctest::Approx(10.0));
	CHECK(s.p50Ms == doctest::Approx(6.0));
}

TEST_CASE("computeFrameStats is order-independent (input copied, not mutated)")
{
	std::vector<float> samples = { 3, 1, 2 };
	FrameStats s = computeFrameStats(samples);
	CHECK(s.minMs == doctest::Approx(1.0));
	CHECK(s.maxMs == doctest::Approx(3.0));
	// caller's vector must be untouched (passed by value)
	CHECK(samples[0] == doctest::Approx(3.0));
	CHECK(samples[2] == doctest::Approx(2.0));
}
