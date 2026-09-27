#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string_view>
#include <vector>

#include "bench_common.hpp"

constexpr double BAL_TOL = 0.45;
constexpr std::size_t LIMIT_SIZE = 50U;
constexpr int SEEDS[5] = {0, 1, 2, 3, 4};

struct SplitMix64 {
    std::uint64_t state;
    explicit SplitMix64(std::uint64_t seed) : state(seed) {}
    auto next() -> std::uint64_t {
        state += 0x9E3779B97F4A7C15ULL;
        std::uint64_t z = state;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
    auto below(std::uint64_t n) -> std::uint64_t { return next() % n; }
};

auto make_init(std::size_t n, std::uint8_t k, int seed) -> std::vector<std::uint8_t> {
    auto rng = SplitMix64{static_cast<std::uint64_t>(seed)};
    auto part = std::vector<std::uint8_t>(n, 0U);
    for (std::size_t i = 0; i != n; ++i) {
        part[i] = static_cast<std::uint8_t>(rng.below(k));
    }
    return part;
}

void emit(const char* testcase, std::uint8_t k, const char* algo, bool ml, int seed, int cost,
          double secs) {
    std::printf(
        "{\"lang\": \"cpp\", \"testcase\": \"%s\", \"k\": %u, \"algo\": \"%s\", \"ml\": %s, "
        "\"seed\": %d, \"cost\": %d, \"time_s\": %.4f}\n",
        testcase, static_cast<unsigned>(k), algo, ml ? "true" : "false", seed, cost, secs);
}

void bench_case(const char* name, const SimpleNetlist& hyprgraph) {
    const auto n = hyprgraph.number_of_modules();
    for (std::uint8_t k : {static_cast<std::uint8_t>(2), static_cast<std::uint8_t>(3)}) {
        for (const char* algo : {"FM", "NN"}) {
            const bool nn = std::string_view{algo} == "NN";
            for (bool ml : {false, true}) {
                for (int seed : SEEDS) {
                    auto init = make_init(n, k, seed);
                    auto part = init;
                    const auto t0 = std::chrono::steady_clock::now();
                    const auto cost = ml ? partition_ml(hyprgraph, part, k, nn, BAL_TOL, LIMIT_SIZE)
                                         : partition_flat(hyprgraph, part, k, nn, BAL_TOL);
                    const auto t1 = std::chrono::steady_clock::now();
                    const auto secs = std::chrono::duration<double>{t1 - t0}.count();
                    emit(name, k, algo, ml, seed, cost, secs);
                }
            }
        }
    }
}

int main() {
    const auto p1 = readNetD("../../testcases/p1.net");
    auto ibm03 = readNetD("../../testcases/ibm03.net");
    readAre(ibm03, "../../testcases/ibm03.are");
    bench_case("p1", p1);
    bench_case("ibm03", ibm03);
    return 0;
}
