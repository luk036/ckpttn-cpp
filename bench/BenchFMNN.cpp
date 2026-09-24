#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string_view>
#include <vector>

#include <netlistx/netlist.hpp>

#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMKWayConstrMgr.hpp>
#include <ckpttn/FMKWayGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/MLPartMgr.hpp>
#include <ckpttn/NNPartMgr.hpp>

extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& hyprgraph, std::string_view areFileName);

using BiFmPM = FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>;
using BiNnPM = NNPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>;
using KwFmPM =
    FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>;
using KwNnPM =
    NNPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>;

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

auto run_flat(const SimpleNetlist& hyprgraph, std::vector<std::uint8_t>& part, std::uint8_t k,
              bool nn) -> int {
    if (k == 2U) {
        FMBiGainMgr<SimpleNetlist> gain_mgr{hyprgraph};
        FMBiConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, BAL_TOL};
        if (nn) {
            BiNnPM part_mgr{hyprgraph, gain_mgr, constr_mgr, 2U};
            part_mgr.legalize(part);
            part_mgr.optimize(part);
            return part_mgr.total_cost;
        }
        BiFmPM part_mgr{hyprgraph, gain_mgr, constr_mgr};
        part_mgr.legalize(part);
        part_mgr.optimize(part);
        return part_mgr.total_cost;
    }
    FMKWayGainMgr<SimpleNetlist> gain_mgr{hyprgraph, k};
    FMKWayConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, BAL_TOL, k};
    if (nn) {
        KwNnPM part_mgr{hyprgraph, gain_mgr, constr_mgr, k};
        part_mgr.legalize(part);
        part_mgr.optimize(part);
        return part_mgr.total_cost;
    }
    KwFmPM part_mgr{hyprgraph, gain_mgr, constr_mgr, k};
    part_mgr.legalize(part);
    part_mgr.optimize(part);
    return part_mgr.total_cost;
}

auto run_ml(const SimpleNetlist& hyprgraph, std::vector<std::uint8_t>& part, std::uint8_t k,
            bool nn) -> int {
    if (k == 2U) {
        MLPartMgr part_mgr{BAL_TOL};
        part_mgr.set_limitsize(LIMIT_SIZE);
        if (nn) {
            part_mgr.run_Partition<SimpleNetlist, BiNnPM>(hyprgraph, part);
        } else {
            part_mgr.run_Partition<SimpleNetlist, BiFmPM>(hyprgraph, part);
        }
        return part_mgr.total_cost;
    }
    MLPartMgr part_mgr{BAL_TOL, k};
    part_mgr.set_limitsize(LIMIT_SIZE);
    if (nn) {
        part_mgr.run_Partition<SimpleNetlist, KwNnPM>(hyprgraph, part);
    } else {
        part_mgr.run_Partition<SimpleNetlist, KwFmPM>(hyprgraph, part);
    }
    return part_mgr.total_cost;
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
    for (std::uint8_t k :
         {static_cast<std::uint8_t>(2), static_cast<std::uint8_t>(3)}) {
        for (const char* algo : {"FM", "NN"}) {
            const bool nn = std::string_view{algo} == "NN";
            for (bool ml : {false, true}) {
                for (int seed : SEEDS) {
                    auto init = make_init(n, k, seed);
                    auto part = init;
                    const auto t0 = std::chrono::steady_clock::now();
                    const auto cost = ml ? run_ml(hyprgraph, part, k, nn)
                                         : run_flat(hyprgraph, part, k, nn);
                    const auto t1 = std::chrono::steady_clock::now();
                    const auto secs =
                        std::chrono::duration<double>{t1 - t0}.count();
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
