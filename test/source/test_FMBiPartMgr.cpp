#include <doctest/doctest.h>  // for ResultBuilder, TestCase, CHECK
// #include <__config>                  // for std
#include <ckpttn/FMBiConstrMgr.hpp>  // for FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>    // for FMBiGainMgr
#include <ckpttn/FMPartMgr.hpp>      // for FMPartMgr
#include <cstdint>                   // for uint8_t
#include <netlistx/netlist.hpp>      // for SimpleNetlist
#include <string_view>               // for std::string_view
#include <vector>                    // for vector

using namespace std;

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hyprgraph, std::string_view areFileName);

/**
 * @brief Run test cases
 *
 * @param[in] hyprgraph
 */
void run_FMBiPartMgr(const SimpleNetlist &hyprgraph) {
    FMBiGainMgr<SimpleNetlist> gain_mgr{hyprgraph};
    FMBiConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, 0.4};
    FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>> part_mgr{
        hyprgraph, gain_mgr, constr_mgr};
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

TEST_CASE("Test FMBiPartMgr") {
    const auto hyprgraph = create_test_netlist();
    run_FMBiPartMgr(hyprgraph);
}

TEST_CASE("Test FMBiPartMgr dwarf") {
    const auto hyprgraph = create_dwarf();
    run_FMBiPartMgr(hyprgraph);
}

TEST_CASE("Test FMBiPartMgr p1") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    run_FMBiPartMgr(hyprgraph);
}

// TEST_CASE("Test FMBiPartMgr ibm01")
// {
//     auto hyprgraph = readNetD("../../testcases/ibm01.net");
//     readAre(hyprgraph, "../../testcases/ibm01.are");
//     run_FMBiPartMgr(hyprgraph);
// }

// TEST_CASE("Test FMBiPartMgr ibm18")
// {
//     auto hyprgraph = readNetD("../../testcases/ibm18.net");
//     readAre(hyprgraph, "../../testcases/ibm18.are");
//     run_FMBiPartMgr(hyprgraph);
// }
