#include <benchmark/benchmark.h>

static void ecs(benchmark::State& state) {
    for (auto _ : state) {}
}

BENCHMARK(ecs);
