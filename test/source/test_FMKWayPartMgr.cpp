#include <doctest/doctest.h>  // for ResultBuilder, TestCase, CHECK
// #include <__config>                    // for std
#include <string_view>  // for std::string_view
#include <ckpttn/FMKWayConstrMgr.hpp>     // for FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>       // for FMKWayGainMgr
#include <ckpttn/FMPartMgr.hpp>           // for FMPartMgr
#include <cstdint>                        // for uint8_t
#include <vector>                         // for vector

#include "ckpttn/netlist.hpp"  // for SimpleNetlist

using namespace std;

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hyprgraph, std::string_view areFileName);

/**
 * @brief Run test cases
 *
 * @param[in] hyprgraph
 * @param[in] num_parts
 */
void run_FMKWayPartMgr(const SimpleNetlist &hyprgraph, uint8_t num_parts) {
    FMKWayGainMgr<SimpleNetlist> gain_mgr{hyprgraph, num_parts};
    FMKWayConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, 0.4, num_parts};
    FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>> part_mgr{
        hyprgraph, gain_mgr, constr_mgr, num_parts};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);

    part_mgr.legalize(part);
    auto totalcostbefore = part_mgr.total_cost;
    part_mgr.optimize(part);
    CHECK(totalcostbefore >= 0);
    CHECK(part_mgr.total_cost <= totalcostbefore);
    CHECK(part_mgr.total_cost >= 0);
    totalcostbefore = part_mgr.total_cost;
    part_mgr.init(part);
    CHECK(part_mgr.total_cost == totalcostbefore);
}

TEST_CASE("Test FMKWayPartMgr") {
    const auto hyprgraph = create_dwarf();
    run_FMKWayPartMgr(hyprgraph, 3);
}

TEST_CASE("Test FMKWayPartMgr p1") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    run_FMKWayPartMgr(hyprgraph, 4);
}

TEST_CASE("Test FMKWayPartMgr ibm01") {
    auto hyprgraph = readNetD("../../testcases/ibm01.net");
    readAre(hyprgraph, "../../testcases/ibm01.are");
    run_FMKWayPartMgr(hyprgraph, 5);
}

// TEST_CASE("Test FMKWayPartMgr ibm18")
// {
//     auto hyprgraph = readNetD("../../testcases/ibm18.net");
//     readAre(hyprgraph, "../../testcases/ibm18.are");
//     run_FMKWayPartMgr(hyprgraph, 3);
// }
