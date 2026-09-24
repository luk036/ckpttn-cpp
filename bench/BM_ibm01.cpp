#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "bench_common.hpp"

using namespace std;
#ifdef _MSC_VER
#    pragma warning(disable : 4244)
#endif

static constexpr int SEED = 47, RUNS = 5;
static constexpr double BAL_TOL = 0.45;
static constexpr size_t LIMIT = 10;

static auto run_fm(const SimpleNetlist& h, span<uint8_t> p, uint8_t k) -> pair<int, double> {
    auto t0 = chrono::steady_clock::now();
    const auto cost = partition_flat(h, p, k, /*nn=*/false, BAL_TOL);
    auto t1 = chrono::steady_clock::now();
    return {cost, chrono::duration<double>(t1 - t0).count()};
}

static auto run_ml(const SimpleNetlist& h, span<uint8_t> p, uint8_t k) -> pair<int, double> {
    auto t0 = chrono::steady_clock::now();
    const auto cost = partition_ml(h, p, k, /*nn=*/false, BAL_TOL, LIMIT);
    auto t1 = chrono::steady_clock::now();
    return {cost, chrono::duration<double>(t1 - t0).count()};
}

int main() {
    struct TC {
        const char* net;
        const char* are;
    };
    TC cs[] = {{.net = "../../testcases/ibm01.net", .are = "../../testcases/ibm01.are"},
               {.net = "../../testcases/p1.net", .are = nullptr}};

    for (auto& c : cs) {
        auto h = readNetD(c.net);
        if (c.are != nullptr) readAre(h, c.are);

        for (auto k : {2, 3, 5}) {
            auto N = h.number_of_modules();
            ankerl::nanobench::Bench bench;
            bench.title(string("BM ibm01 k=") + to_string(k)).unit("op").warmup(1).epochs(RUNS);

            bench.run("FM", [&] {
                mt19937 rg(SEED);
                auto pt = vector<uint8_t>(N, 0);
                if (k == 2) {
                    bernoulli_distribution d(0.5);
                    for (size_t i = 0; i < N; ++i) pt[i] = static_cast<uint8_t>(d(rg));
                } else {
                    uniform_int_distribution<int> d(0, k - 1);
                    for (size_t i = 0; i < N; ++i) pt[i] = static_cast<uint8_t>(d(rg));
                }
                auto result = run_fm(h, pt, k);
                ankerl::nanobench::doNotOptimizeAway(result);
            });

            bench.run("ML", [&] {
                mt19937 rg(SEED);
                auto pt = vector<uint8_t>(N, 0);
                if (k == 2) {
                    bernoulli_distribution d(0.5);
                    for (size_t i = 0; i < N; ++i) pt[i] = static_cast<uint8_t>(d(rg));
                } else {
                    uniform_int_distribution<int> d(0, k - 1);
                    for (size_t i = 0; i < N; ++i) pt[i] = static_cast<uint8_t>(d(rg));
                }
                auto result = run_ml(h, pt, k);
                ankerl::nanobench::doNotOptimizeAway(result);
            });
        }
    }
}
