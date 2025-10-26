#include "test_common.hpp"
#include <ckpttn/FMKWayConstrMgr.hpp>  // for FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>    // for FMKWayGainMgr
#include <netlistx/netlist.hpp>        // for SimpleNetlist
 
TEST_CASE("Test FMKWayPartMgr") {
    const auto hyprgraph = create_dwarf();
    run_PartMgr<FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>(hyprgraph, 3);
}
 
TEST_CASE("Test FMKWayPartMgr p1") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    run_PartMgr<FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>(hyprgraph, 4);
}
 
TEST_CASE("Test FMKWayPartMgr ibm01") {
    auto hyprgraph = readNetD("../../testcases/ibm01.net");
    readAre(hyprgraph, "../../testcases/ibm01.are");
    run_PartMgr<FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>(hyprgraph, 5);
}

// TEST_CASE("Test FMKWayPartMgr ibm18")
// {
//     auto hyprgraph = readNetD("../../testcases/ibm18.net");
//     readAre(hyprgraph, "../../testcases/ibm18.are");
//     run_FMKWayPartMgr(hyprgraph, 3);
// }
