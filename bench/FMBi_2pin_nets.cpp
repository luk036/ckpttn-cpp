#include <boost/utility/string_view.hpp> // for boost::string_view
#include <ckpttn/FMBiConstrMgr.hpp>      // for FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>        // for FMBiGainMgr
#include <ckpttn/FMPartMgr.hpp>          // for FMPartMgr
#include <cstdint>                       // for uint8_t
#include <vector>                        // for vector

#include "benchmark/benchmark.h"   // for BENCHMARK, State, BENCHMARK_MAIN
#include "ckpttn/FMBiGainCalc.hpp" // for FMBiGainCalc
#include "ckpttn/netlist.hpp"      // for SimpleNetlist

extern auto create_test_netlist()
    -> SimpleNetlist;                        // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist; // import create_dwarf
extern auto readNetD(boost::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hgr, boost::string_view areFileName);

/**
 * @brief run FM Bi-partitioning
 *
 * @param[in] hgr
 * @param[in] option
 */
void run_FMBiPartMgr(const SimpleNetlist &hgr, bool option) {
    FMBiGainMgr<SimpleNetlist> gain_mgr{hgr};
    gain_mgr.gain_calc.special_handle_2pin_nets = option;

    FMBiConstrMgr<SimpleNetlist> constr_mgr{hgr, 0.45};
    FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>,
              FMBiConstrMgr<SimpleNetlist>>
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
        run_FMBiPartMgr(hgr, true);
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
        run_FMBiPartMgr(hgr, false);
    }
}
BENCHMARK(BM_without_2pin_nets);

BENCHMARK_MAIN();

/*
2: ---------------------------------------------------------------
2: Benchmark                     Time             CPU   Iterations
2: ---------------------------------------------------------------
2: BM_with_2pin_nets      86441084 ns     86440541 ns            8
2: BM_without_2pin_nets   95729431 ns     95728056 ns            7
2/4 Test #2: Bench_FMBi_2pin_nets .............   Passed    1.70 sec
*/