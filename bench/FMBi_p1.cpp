#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <cstdint>
#include <netlistx/netlist.hpp>
#include <string_view>
#include <vector>

#include "benchmark/benchmark.h"
#include "ckpttn/FMBiGainCalc.hpp"

extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& hyprgraph, std::string_view areFileName);

void run_FMBiPartMgr(const SimpleNetlist& hyprgraph, bool option) {
    FMBiGainMgr<SimpleNetlist> gain_mgr{hyprgraph};
    gain_mgr.gain_calc.special_handle_2pin_nets = option;
    FMBiConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, 0.45};
    FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>> part_mgr{hyprgraph, gain_mgr, constr_mgr};
    std::vector<std::uint8_t> part(hyprgraph.number_of_modules(), 0);
    part_mgr.legalize(part);
    part_mgr.optimize(part);
}

static void BM_p1_with_2pin_nets(benchmark::State& state) {
    auto hyprgraph = readNetD("../../testcases/p1.net");
    while (state.KeepRunning()) {
        run_FMBiPartMgr(hyprgraph, true);
    }
}
BENCHMARK(BM_p1_with_2pin_nets);

static void BM_p1_without_2pin_nets(benchmark::State& state) {
    auto hyprgraph = readNetD("../../testcases/p1.net");
    while (state.KeepRunning()) {
        run_FMBiPartMgr(hyprgraph, false);
    }
}
BENCHMARK(BM_p1_without_2pin_nets);

BENCHMARK_MAIN();
