#pragma once
// Header-only micro-benchmark harness: run named cases, time them, exit nonzero if any exceeds its budget.
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace advbench
{
	struct Case
	{
		std::string name;
		long iterations;
		double budgetMs; // ceiling for the whole `iterations` run (with headroom, not a target)
		std::function<void()> body;
	};

	class Registry
	{
	public:
		static Registry& get()
		{
			static Registry r;
			return r;
		}

		void add(Case c) { m_cases.push_back(std::move(c)); }

		// Returns the number of cases that blew their budget (0 = all within budget).
		int runAll()
		{
			using clock = std::chrono::steady_clock;
			std::printf("%-30s %10s %10s %10s %10s   %s\n",
			            "case",
			            "iters",
			            "total ms",
			            "us/it",
			            "budget ms",
			            "result");
			std::printf("--------------------------------------------------------------------------------\n");

			int failures = 0;
			for (auto& c : m_cases)
			{
				const long warm = std::min<long>(c.iterations, 1000);
				for (long i = 0; i < warm; ++i)
					c.body();

				const auto t0 = clock::now();
				for (long i = 0; i < c.iterations; ++i)
					c.body();
				const double ms =
				    std::chrono::duration<double, std::milli>(clock::now() - t0).count();

				const double usPerIt = (c.iterations > 0) ? (ms / c.iterations * 1000.0) : 0.0;
				const bool ok = ms <= c.budgetMs;
				if (!ok)
					++failures;

				std::printf("%-30s %10ld %10.2f %10.3f %10.2f   %s\n",
				            c.name.c_str(),
				            c.iterations,
				            ms,
				            usPerIt,
				            c.budgetMs,
				            ok ? "OK" : "OVER BUDGET");
			}

			std::printf("--------------------------------------------------------------------------------\n");
			std::printf("%d/%zu cases over budget\n", failures, m_cases.size());
			return failures;
		}

	private:
		std::vector<Case> m_cases;
	};

	struct AutoReg
	{
		explicit AutoReg(Case c) { Registry::get().add(std::move(c)); }
	};

	// Defeat dead-code elimination for trivial bench bodies.
	template <class T>
	inline void keep(T&& v)
	{
		static volatile std::size_t sink = 0;
		sink ^= reinterpret_cast<std::size_t>(&v);
		(void)sink; // read the volatile so it counts as used (defeats DCE + the warning)
	}
} // namespace advbench

// Define a benchmark case. Body runs once per iteration; the runner loops `iters` times.
#define ADV_BENCH(NAME, ITERS, BUDGET_MS)                                                               \
	static void NAME();                                                                                 \
	static ::advbench::AutoReg adv_autoreg_##NAME{::advbench::Case{#NAME, (ITERS), (BUDGET_MS), NAME}}; \
	static void NAME()
