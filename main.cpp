#include <benchmark/benchmark.h>
#include <immintrin.h>
#include <vector>
#include <random>
#include <functional>

using value_t = uint32_t;

static std::vector<value_t> CreateRandomValues(size_t size) {
	std::vector<value_t> values;
	values.resize(size);
	std::mt19937 rng;
	std::uniform_int_distribution<value_t> dist;
	std::generate(values.begin(), values.end(), std::bind(dist, rng));
	return values;
}

static constexpr size_t VALUE_COUNT = (256 * 1024 * 1024) / sizeof(value_t); // 256 MB
static constexpr size_t ITERATIONS = 1'000;

#ifndef _WIN32

template<typename T>
static T ReadGSDirect(uint32_t offset) {
	T res;
	asm("mov %%gs:(%[offset]), %[res]"
		: [res] "=r" (res)
		: [offset] "r" (offset * sizeof(T)));
	return res;
}

static void BM_ReadGSDirect(benchmark::State& state) {

	auto const values = CreateRandomValues(VALUE_COUNT);
	std::mt19937 rng;
	std::uniform_int_distribution<value_t> dist{ 0, VALUE_COUNT - 1 };
	_writegsbase_u64((uintptr_t)values.data());
	std::vector<size_t> offsets;
	offsets.resize(ITERATIONS);
	assert((value_t*)_readgsbase_u64() == values.data());
	assert(values[VALUE_COUNT / 2] == ReadGSDirect<value_t>(VALUE_COUNT / 2));

	for (auto _ : state) {
		state.PauseTiming();
		std::generate(offsets.begin(), offsets.end(), std::bind(dist, rng));
		state.ResumeTiming();

		for (uint32_t i = 0; i < ITERATIONS; ++i) {
			benchmark::DoNotOptimize(ReadGSDirect<value_t>(offsets[i]));
		}
	}

	state.SetItemsProcessed(state.iterations() * ITERATIONS);
	state.SetBytesProcessed(state.iterations() * ITERATIONS * sizeof(uint32_t));
}

// Benchmarks randomized reads (eliminates CPU data caches) directly from the current GS base address with offset
BENCHMARK(BM_ReadGSDirect);



template<typename T>
static T ReadGSBase(size_t offset) {
	return *(value_t*)((char*)_readgsbase_u64() + offset * sizeof(value_t));
}

static void BM_ReadGSBase(benchmark::State& state) {
	auto const values = CreateRandomValues(VALUE_COUNT);
	std::mt19937 rng;
	std::uniform_int_distribution<value_t> dist{ 0, VALUE_COUNT - 1 };
	_writegsbase_u64((uintptr_t)values.data());
	std::vector<size_t> offsets;
	offsets.resize(ITERATIONS);
	assert((value_t*)_readgsbase_u64() == values.data());
	assert(values[VALUE_COUNT / 2] == ReadGSBase<value_t>(VALUE_COUNT / 2));

	for (auto _ : state) {
		state.PauseTiming();
		std::generate(offsets.begin(), offsets.end(), std::bind(dist, rng));
		state.ResumeTiming();

		for (uint32_t i = 0; i < ITERATIONS; ++i) {
			benchmark::DoNotOptimize(ReadGSBase<value_t>(offsets[i]));
		}
	}

	state.SetItemsProcessed(state.iterations() * ITERATIONS);
	state.SetBytesProcessed(state.iterations() * ITERATIONS * sizeof(uint32_t));
}

// Benchmarks randomized reads (eliminates CPU data caches) from the current GS base address (read using RDGSBASE) and manual offset computation
BENCHMARK(BM_ReadGSBase);
#endif

thread_local value_t const* values_base;

template<typename T>
static T ReadTLS(size_t offset) {
	return values_base[offset];
}

static void BM_ReadTLS(benchmark::State& state) {
	auto const values = CreateRandomValues(VALUE_COUNT);
	std::mt19937 rng;
	std::uniform_int_distribution<value_t> dist{ 0, VALUE_COUNT - 1 };
	values_base = values.data();
	std::vector<size_t> offsets;
	offsets.resize(ITERATIONS);
	benchmark::DoNotOptimize(values_base);
	assert(values_base == values.data());
	assert(values[VALUE_COUNT / 2] == values_base[VALUE_COUNT / 2]);

	for (auto _ : state) {
		state.PauseTiming();
		std::generate(offsets.begin(), offsets.end(), std::bind(dist, rng));
		state.ResumeTiming();

		for (uint32_t i = 0; i < ITERATIONS; ++i) {
			benchmark::DoNotOptimize(ReadTLS<value_t>(offsets[i]));
		}
	}

	state.SetItemsProcessed(state.iterations() * ITERATIONS);
	state.SetBytesProcessed(state.iterations() * ITERATIONS * sizeof(uint32_t));
}

// Benchmarks randomized reads (eliminates CPU data caches) from the current base address stored in TLS
BENCHMARK(BM_ReadTLS);

static void BM_Read(benchmark::State& state) {
	auto const values = CreateRandomValues(VALUE_COUNT);
	std::mt19937 rng;
	std::uniform_int_distribution<value_t> dist{ 0, VALUE_COUNT - 1 };
	std::vector<size_t> offsets;
	offsets.resize(ITERATIONS);

	for (auto _ : state) {
		state.PauseTiming();
		std::generate(offsets.begin(), offsets.end(), std::bind(dist, rng));
		state.ResumeTiming();

		for (uint32_t i = 0; i < ITERATIONS; ++i) {
			benchmark::DoNotOptimize(values[offsets[i]]);
		}
	}

	state.SetItemsProcessed(state.iterations() * ITERATIONS);
	state.SetBytesProcessed(state.iterations() * ITERATIONS * sizeof(uint32_t));
}

// Benchmarks randomized reads (eliminates CPU data caches) from a vector directly (reference benchmark)
BENCHMARK(BM_Read);
