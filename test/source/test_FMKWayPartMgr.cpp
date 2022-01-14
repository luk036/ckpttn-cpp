#include <doctest/doctest.h>  // for ResultBuilder, TestCase, CHECK
// #include <__config>                    // for std
#include <boost/utility/string_view.hpp>  // for boost::string_view
#include <ckpttn/FMKWayConstrMgr.hpp>     // for FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>       // for FMKWayGainMgr
#include <ckpttn/FMPartMgr.hpp>           // for FMPartMgr
#include <cstdint>                        // for uint8_t
#include <vector>                         // for vector

#include "ckpttn/netlist.hpp"  // for SimpleNetlist

using namespace std;

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(boost::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& H, boost::string_view areFileName);

/**
 * @brief Run test cases
 *
 * @param[in] H
 * @param[in] K
 */
void run_FMKWayPartMgr(const SimpleNetlist& H, uint8_t K) {
    FMKWayGainMgr<SimpleNetlist> gainMgr{H, K};
    FMKWayConstrMgr<SimpleNetlist> constrMgr{H, 0.4, K};
    FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>> partMgr{
        H, gainMgr, constrMgr, K};
    vector<uint8_t> part(H.number_of_modules(), 0);

    partMgr.legalize(part);
    auto totalcostbefore = partMgr.totalcost;
    partMgr.optimize(part);
    CHECK(totalcostbefore >= 0);
    CHECK(partMgr.totalcost <= totalcostbefore);
    CHECK(partMgr.totalcost >= 0);
    totalcostbefore = partMgr.totalcost;
    partMgr.init(part);
    CHECK(partMgr.totalcost == totalcostbefore);
}

TEST_CASE("Test FMKWayPartMgr") {
    const auto H = create_dwarf();
    run_FMKWayPartMgr(H, 3);
}

TEST_CASE("Test FMKWayPartMgr p1") {
    const auto H = readNetD("../../testcases/p1.net");
    run_FMKWayPartMgr(H, 3);
}

TEST_CASE("Test FMKWayPartMgr ibm01") {
    auto H = readNetD("../../testcases/ibm01.net");
    readAre(H, "../../testcases/ibm01.are");
    run_FMKWayPartMgr(H, 3);
}

// TEST_CASE("Test FMKWayPartMgr ibm18")
// {
//     auto H = readNetD("../../testcases/ibm18.net");
//     readAre(H, "../../testcases/ibm18.are");
//     run_FMKWayPartMgr(H, 3);
// }
