#include <ckpttn/FMBiConstrMgr.hpp> // import FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>   // import FMBiGainMgr
#include <ckpttn/FMPartMgr.hpp>     // import FMBiPartMgr
#include <doctest/doctest.h>
#include <string_view>

extern auto create_test_netlist()
    -> SimpleNetlist;                        // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist; // import create_dwarf
extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& H, std::string_view areFileName);

/**
 * @brief Run test cases
 *
 * @param[in] H
 */
void run_FMBiPartMgr(const SimpleNetlist& H)
{
    auto gainMgr = FMBiGainMgr {H};
    auto constrMgr = FMBiConstrMgr {H, 0.4};
    auto partMgr =
        FMPartMgr<FMBiGainMgr, FMBiConstrMgr> {H, gainMgr, constrMgr};
    auto part = std::vector<std::uint8_t>(H.number_of_modules(), 0);
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

TEST_CASE("Test FMBiPartMgr")
{
    const auto H = create_test_netlist();
    run_FMBiPartMgr(H);
}

TEST_CASE("Test FMBiPartMgr dwarf")
{
    const auto H = create_dwarf();
    run_FMBiPartMgr(H);
}

TEST_CASE("Test FMBiPartMgr p1")
{
    const auto H = readNetD("../../testcases/p1.net");
    run_FMBiPartMgr(H);
}

// TEST_CASE("Test FMBiPartMgr ibm01")
// {
//     auto H = readNetD("../../testcases/ibm01.net");
//     readAre(H, "../../testcases/ibm01.are");
//     run_FMBiPartMgr(H);
// }

// TEST_CASE("Test FMBiPartMgr ibm18")
// {
//     auto H = readNetD("../../testcases/ibm18.net");
//     readAre(H, "../../testcases/ibm18.are");
//     run_FMBiPartMgr(H);
// }
