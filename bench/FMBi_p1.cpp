#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <cstdint>
#include <netlistx/netlist.hpp>
#include <string_view>
#include <vector>

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include "ckpttn/FMBiGainCalc.hpp"

extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& hyprgraph, std::string_view areFileName);

void run_FMBiPartMgr(const SimpleNetlist& hyprgraph, bool option) {
    FMBiGainMgr<SimpleNetlist> gain_mgr{hyprgraph};
    gain_mgr.gain_calc.special_handle_2pin_nets = option;
    FMBiConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, 0.45};
    FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>> part_mgr{
        hyprgraph, gain_mgr, constr_mgr};
    std::vector<std::uint8_t> part(hyprgraph.number_of_modules(), 0);
    part_mgr.legalize(part);
    part_mgr.optimize(part);
}

int main() {
    ankerl::nanobench::Bench bench;
    bench.title("FMBi p1").unit("op").warmup(10).epochs(10);

    auto hyprgraph_with = readNetD("../../testcases/p1.net");
    auto hyprgraph_without = readNetD("../../testcases/p1.net");

    bench.run("BM_p1_with_2pin_nets", [&] {
        run_FMBiPartMgr(hyprgraph_with, true);
        ankerl::nanobench::doNotOptimizeAway(hyprgraph_with);
    });

    bench.run("BM_p1_without_2pin_nets", [&] {
        run_FMBiPartMgr(hyprgraph_without, false);
        ankerl::nanobench::doNotOptimizeAway(hyprgraph_without);
    });
}
