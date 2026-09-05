#include <benchmark/benchmark.h>

static auto ecs(benchmark::State& state) -> void {
    for (auto _ : state) {}
}

BENCHMARK(ecs);
