#include <boost/utility/string_view.hpp>  // for boost::string_view
#include <ckpttn/FMKWayConstrMgr.hpp>     // for FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>       // for FMKWayGainMgr
#include <ckpttn/FMPartMgr.hpp>           // for FMPartMgr
#include <cstdint>                        // for uint8_t
#include <vector>                         // for vector

#include "benchmark/benchmark.h"      // for BENCHMARK, State, BENCHMARK_MAIN
#include "ckpttn/FMKWayGainCalc.hpp"  // for FMKWayGainCalc
#include "ckpttn/netlist.hpp"         // for SimpleNetlist

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(boost::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hyprgraph, boost::string_view areFileName);

/**
 * The function `run_FMKWayPartMgr` runs the Fiduccia-Mattheyses num_parts-way partitioning
 * algorithm on a given netlist.
 *
 * @param[in] hyprgraph The parameter `hyprgraph` is a reference to an object of type
 * `SimpleNetlist`, which represents a netlist (a hypergraph representation of a circuit).
 * @param[in] num_parts The `num_parts` parameter represents the number of partitions or groups that
 * the Fiduccia-Mattheyses algorithm will create. It determines how many parts the netlist will be
 * divided into.
 * @param[in] option The "option" parameter is a boolean flag that determines whether a special
 * handling for 2-pin nets should be applied during the gain calculation.
 */
void run_FMKWayPartMgr(SimpleNetlist &hyprgraph, std::uint8_t num_parts, bool option) {
    FMKWayGainMgr<SimpleNetlist> gain_mgr{hyprgraph, num_parts};
    gain_mgr.gain_calc.special_handle_2pin_nets = option;

    FMKWayConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, 0.4, num_parts};
    FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>> part_mgr{
        hyprgraph, gain_mgr, constr_mgr};
    std::vector<std::uint8_t> part(hyprgraph.number_of_modules(), 0);

    part_mgr.legalize(part);
    // auto totalcostbefore = part_mgr.total_cost;
    part_mgr.optimize(part);
    // CHECK(totalcostbefore >= 0);
    // CHECK(part_mgr.total_cost <= totalcostbefore);
    // CHECK(part_mgr.total_cost >= 0);
}

/**
 * @brief
 *
 * @param[in] state
 */
static void BM_with_2pin_nets(benchmark::State &state) {
    auto hyprgraph = readNetD("../../testcases/ibm03.net");
    readAre(hyprgraph, "../../testcases/ibm03.are");

    while (state.KeepRunning()) {
        run_FMKWayPartMgr(hyprgraph, 3, true);
    }
}

// Register the function as a benchmark
BENCHMARK(BM_with_2pin_nets);

//~~~~~~~~~~~~~~~~

/**
 * @brief Define another benchmark
 *
 * @param[in] state
 */
static void BM_without_2pin_nets(benchmark::State &state) {
    auto hyprgraph = readNetD("../../testcases/ibm03.net");
    readAre(hyprgraph, "../../testcases/ibm03.are");

    while (state.KeepRunning()) {
        run_FMKWayPartMgr(hyprgraph, 3, false);
    }
}
BENCHMARK(BM_without_2pin_nets);

BENCHMARK_MAIN();

/*
3: ---------------------------------------------------------------
3: Benchmark                     Time             CPU   Iterations
3: ---------------------------------------------------------------
3: BM_with_2pin_nets     463584957 ns    463577351 ns            2
3: BM_without_2pin_nets  492702676 ns    492693494 ns            2
3/4 Test #3: Bench_FMKWay_2pin_nets ...........   Passed    3.02 sec
*/