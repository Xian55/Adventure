#include "Bench.h"

// Perf gate entry point: returns the number of cases over budget, so CTest/CI fails on regression.
int main()
{
	return advbench::Registry::get().runAll();
}
