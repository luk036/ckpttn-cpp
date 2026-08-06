#include <ckpttn/FMBiConstrMgr.hpp>  // for FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>    // for FMBiGainMgr
#include <ckpttn/FMPartMgr.hpp>      // for FMPartMgr
#include <cstdint>                   // for uint8_t
#include <netlistx/netlist.hpp>      // for SimpleNetlist
#include <string_view>               // for std::string_view
#include <vector>                    // for vector

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include "ckpttn/FMBiGainCalc.hpp"  // for FMBiGainCalc

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& hyprgraph, std::string_view areFileName);

/**
 * The function "run_FMBiPartMgr" runs the Fiduccia-Mattheyses Bi-partitioning algorithm on a given
 * netlist.
 *
 * @param[in] hyprgraph The parameter `hyprgraph` is of type `SimpleNetlist` and represents a
 * netlist, which is a data structure that describes the connections between modules in a circuit
 * design.
 * @param[in] option The "option" parameter is a boolean value that determines whether a special
 * handling for 2-pin nets should be applied during the gain calculation.
 */
void run_FMBiPartMgr(const SimpleNetlist& hyprgraph, bool option) {
    FMBiGainMgr<SimpleNetlist> gain_mgr{hyprgraph};
    gain_mgr.gain_calc.special_handle_2pin_nets = option;

    FMBiConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, 0.45};
    FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>> part_mgr{
        hyprgraph, gain_mgr, constr_mgr};
    std::vector<std::uint8_t> part(hyprgraph.number_of_modules(), 0);
    part_mgr.legalize(part);
    // auto totalcostbefore = part_mgr.total_cost;
    part_mgr.optimize(part);
    // CHECK_GE(totalcostbefore, 0);
    // CHECK_LE(part_mgr.total_cost, totalcostbefore);
    // CHECK_GE(part_mgr.total_cost, 0);
}

int main() {
    ankerl::nanobench::Bench bench;
    bench.title("FMBi 2-pin nets").unit("op").warmup(10).epochs(10);

    auto hyprgraph_with = readNetD("../../testcases/ibm03.net");
    readAre(hyprgraph_with, "../../testcases/ibm03.are");

    auto hyprgraph_without = readNetD("../../testcases/ibm03.net");
    readAre(hyprgraph_without, "../../testcases/ibm03.are");

    bench.run("BM_with_2pin_nets", [&] {
        run_FMBiPartMgr(hyprgraph_with, true);
        ankerl::nanobench::doNotOptimizeAway(hyprgraph_with);
    });

    bench.run("BM_without_2pin_nets", [&] {
        run_FMBiPartMgr(hyprgraph_without, false);
        ankerl::nanobench::doNotOptimizeAway(hyprgraph_without);
    });
}

/*
2: ---------------------------------------------------------------
2: Benchmark                     Time             CPU   Iterations
2: ---------------------------------------------------------------
2: BM_with_2pin_nets      86441084 ns     86440541 ns            8
2: BM_without_2pin_nets   95729431 ns     95728056 ns            7
2/4 Test #2: Bench_FMBi_2pin_nets .............   Passed    1.70 sec
*/