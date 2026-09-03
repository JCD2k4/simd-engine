#include <vector>
#include <random>
#include <xmmintrin.h>

#include "benchmark/benchmark.h"

static std::vector<float> make_random_floats(int n) {
    std::vector<float> v(n);
    std::mt19937 gen(42);                                   // fixed seed = reproducible benchmarks
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (auto& x : v) x = dist(gen);
    return v;
}

void BM_SCALAR(benchmark::State& state){
    
    const int N = state.range(0);
    std::vector<float> ptr = make_random_floats(N);

    for (auto _ : state){
        for (int i = 0; i < N; ++i){
            const float f = ptr[i];
            ptr[i] = f * f;
        }
        benchmark::DoNotOptimize(ptr.data());
        benchmark::ClobberMemory();

    }
}

void BM_VECTOR(benchmark::State& state){
    const int N = state.range(0);
    std::vector<float> ptr = make_random_floats(N);

    for (auto _ : state){
        int i = 0;
        for (; i + 4 <= N; i += 4){
            __m128 f = _mm_loadu_ps(ptr.data() + i);
            f = _mm_mul_ps(f, f);
            _mm_storeu_ps(ptr.data() + i, f);
        }
        // handle any leftover elements (N not a multiple of 4)
        for (; i < N; ++i){
            ptr[i] = ptr[i] * ptr[i];
        }
        benchmark::DoNotOptimize(ptr.data());
        benchmark::ClobberMemory();
    }
}


BENCHMARK(BM_SCALAR)->Arg(1 << 22);
BENCHMARK(BM_VECTOR)->Arg(1 << 22);
BENCHMARK_MAIN();