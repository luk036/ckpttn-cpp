#include "test_common.hpp"
#include <ckpttn/FMBiConstrMgr.hpp>  // for FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>    // for FMBiGainMgr
#include <netlistx/netlist.hpp>      // for SimpleNetlist

TEST_CASE("Test FMBiPartMgr") {
    const auto hyprgraph = create_test_netlist();
    run_PartMgr<FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>(hyprgraph);
}

TEST_CASE("Test FMBiPartMgr dwarf") {
    const auto hyprgraph = create_dwarf();
    run_PartMgr<FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>(hyprgraph);
}

TEST_CASE("Test FMBiPartMgr p1") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    run_PartMgr<FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>(hyprgraph);
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
