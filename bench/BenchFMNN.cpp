#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <string_view>

#include "bench_common.hpp"

constexpr double BAL_TOL = 0.45;
constexpr std::size_t LIMIT_SIZE = 50U;
constexpr int SEEDS[5] = {0, 1, 2, 3, 4};
constexpr unsigned STARTS[] = {1U, 2U, 4U, 8U};
constexpr unsigned NUM_THREADS = 10U;

void emit(const char* testcase, std::uint8_t k, const char* algo, bool ml, int seed,
          unsigned starts, int cost, double secs, double cpu_secs) {
    std::printf(
        "{\"lang\": \"cpp\", \"testcase\": \"%s\", \"k\": %u, \"algo\": \"%s\", \"ml\": %s, "
        "\"seed\": %d, \"starts\": %u, \"cost\": %d, \"time_s\": %.4f, \"cpu_s\": %.4f}\n",
        testcase, static_cast<unsigned>(k), algo, ml ? "true" : "false", seed, starts, cost, secs,
        cpu_secs);
}

void bench_case(const char* name, const SimpleNetlist& hyprgraph, xnetwork::thread_pool& pool) {
    for (std::uint8_t k : {static_cast<std::uint8_t>(2), static_cast<std::uint8_t>(3)}) {
        for (const char* algo : {"FM", "NN"}) {
            const bool nn = std::string_view{algo} == "NN";
            for (bool ml : {false, true}) {
                for (int seed : SEEDS) {
                    for (unsigned starts : STARTS) {
                        const auto c0 = std::clock();
                        const auto t0 = std::chrono::steady_clock::now();
                        const auto cost = multi_start_cost(hyprgraph, k, nn, ml, BAL_TOL,
                                                           LIMIT_SIZE, std::nullopt, starts,
                                                           static_cast<std::uint32_t>(seed), pool);
                        const auto t1 = std::chrono::steady_clock::now();
                        const auto c1 = std::clock();
                        const auto secs = std::chrono::duration<double>{t1 - t0}.count();
                        const auto cpu_secs = static_cast<double>(c1 - c0) / CLOCKS_PER_SEC;
                        emit(name, k, algo, ml, seed, starts, cost, secs, cpu_secs);
                    }
                }
            }
        }
    }
}

int main() {
    const auto p1 = readNetD("../../testcases/p1.net");
    auto ibm03 = readNetD("../../testcases/ibm03.net");
    readAre(ibm03, "../../testcases/ibm03.are");
    xnetwork::thread_pool pool(NUM_THREADS);
    bench_case("p1", p1, pool);
    bench_case("ibm03", ibm03, pool);
    return 0;
}
