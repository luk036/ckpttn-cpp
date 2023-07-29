#include <boost/utility/string_view.hpp> // for boost::string_view
#include <ckpttn/FMKWayConstrMgr.hpp>    // for FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>      // for FMKWayGainMgr
#include <ckpttn/FMPartMgr.hpp>          // for FMPartMgr
#include <cstdint>                       // for uint8_t
#include <vector>                        // for vector

#include "benchmark/benchmark.h"     // for BENCHMARK, State, BENCHMARK_MAIN
#include "ckpttn/FMKWayGainCalc.hpp" // for FMKWayGainCalc
#include "ckpttn/netlist.hpp"        // for SimpleNetlist

extern auto create_test_netlist()
    -> SimpleNetlist;                        // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist; // import create_dwarf
extern auto readNetD(boost::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hgr, boost::string_view areFileName);

/**
 * @brief Run FM num_parts-way partitioning
 *
 * @param[in] hgr
 * @param[in] num_parts
 * @param[in] option
 */
void run_FMKWayPartMgr(SimpleNetlist &hgr, std::uint8_t num_parts,
                       bool option) {
    FMKWayGainMgr<SimpleNetlist> gain_mgr{hgr, num_parts};
    gain_mgr.gain_calc.special_handle_2pin_nets = option;

    FMKWayConstrMgr<SimpleNetlist> constr_mgr{hgr, 0.4, num_parts};
    FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>,
              FMKWayConstrMgr<SimpleNetlist>>
        part_mgr{hgr, gain_mgr, constr_mgr};
    std::vector<std::uint8_t> part(hgr.number_of_modules(), 0);

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
    auto hgr = readNetD("../../testcases/ibm03.net");
    readAre(hgr, "../../testcases/ibm03.are");

    while (state.KeepRunning()) {
        run_FMKWayPartMgr(hgr, 3, true);
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
    auto hgr = readNetD("../../testcases/ibm03.net");
    readAre(hgr, "../../testcases/ibm03.are");

    while (state.KeepRunning()) {
        run_FMKWayPartMgr(hgr, 3, false);
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