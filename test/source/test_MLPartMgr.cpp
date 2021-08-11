#include <doctest/doctest.h>

#include <chrono>
#include <ckpttn/FMBiConstrMgr.hpp>    // import FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>      // import FMBiGainMgr
#include <ckpttn/FMKWayConstrMgr.hpp>  // import FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>    // import FMKWayGainMgr
#include <ckpttn/FMPartMgr.hpp>        // import FMBiPartMgr
#include <ckpttn/MLPartMgr.hpp>        // import MLBiPartMgr
// #include <experimental/random>
#include <iostream>
#include <string_view>

// using std::experimental::randint;

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& H, std::string_view areFileName);

TEST_CASE("Test MLBiPartMgr dwarf") {
    const auto H = create_dwarf();
    auto partMgr = MLPartMgr{0.3};
    auto part = std::vector<std::uint8_t>(H.number_of_modules(), 0);
    partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr>>(H, part);
    CHECK(partMgr.totalcost == 2U);
}

TEST_CASE("Test MLKWayPartMgr dwarf") {
    const auto H = create_dwarf();
    auto partMgr = MLPartMgr{0.4, 3};  // 0.3???
    auto part = std::vector<std::uint8_t>(H.number_of_modules(), 0);
    partMgr.run_FMPartition<FMPartMgr<FMKWayGainMgr, FMKWayConstrMgr>>(H, part);
    CHECK(partMgr.totalcost == 4U);
}

TEST_CASE("Test MLBiPartMgr p1") {
    const auto H = readNetD("../../testcases/p1.net");
    auto partMgr = MLPartMgr{0.3};
    partMgr.set_limitsize(500);

    auto mincost = 1000;
    for (auto i = 0; i != 10; ++i) {
        auto part = std::vector<std::uint8_t>(H.number_of_modules(), 0);
        auto whichPart = 0U;
        for (auto& elem : part) {
            whichPart ^= 1;
            elem = std::uint8_t(whichPart);
        }
        partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr>>(H, part);
        if (mincost > partMgr.totalcost) {
            mincost = partMgr.totalcost;
        }
    }
    // CHECK(partMgr.totalcost >= 50);
    // CHECK(partMgr.totalcost <= 50);
    CHECK(mincost >= 29U);
    CHECK(mincost <= 73U);
}

TEST_CASE("Test MLBiPartMgr ibm01") {
    auto H = readNetD("../../testcases/ibm01.net");
    readAre(H, "../../testcases/ibm01.are");
    auto partMgr = MLPartMgr{0.4};
    partMgr.set_limitsize(400);

    auto mincost = 1000;
    for (auto i = 0; i != 10; ++i) {
        auto part = std::vector<std::uint8_t>(H.number_of_modules(), 0);
        auto whichPart = 0U;
        for (auto& elem : part) {
            whichPart ^= 1;
            elem = std::uint8_t(whichPart);
        }
        partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr>>(H, part);
        if (mincost > partMgr.totalcost) {
            mincost = partMgr.totalcost;
        }
    }
    // CHECK(partMgr.totalcost >= 650);
    // CHECK(partMgr.totalcost <= 650);
    CHECK(mincost >= 221U);
    CHECK(mincost <= 695U);
}

TEST_CASE("Test MLBiPartMgr ibm03") {
    auto H = readNetD("../../testcases/ibm03.net");
    readAre(H, "../../testcases/ibm03.are");
    auto partMgr = MLPartMgr{0.45};
    partMgr.set_limitsize(300);
    auto part = std::vector<std::uint8_t>(H.number_of_modules(), 0);
    // auto part_info = PartInfo{std::move(part), py::set<node_t>()};
    auto begin = std::chrono::steady_clock::now();
    partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr>>(H, part);
    std::chrono::duration<double> last = std::chrono::steady_clock::now() - begin;
    std::cout << "time: " << last.count() << std::endl;
    CHECK(partMgr.totalcost >= 1104U);
    CHECK(partMgr.totalcost <= 2041U);
}

TEST_CASE("Test MLBiPartMgr ibm18") {
    auto H = readNetD("../../testcases/ibm18.net");
    readAre(H, "../../testcases/ibm18.are");
    auto partMgr = MLPartMgr{0.45};
    partMgr.set_limitsize(24000);
    auto part = std::vector<std::uint8_t>(H.number_of_modules(), 0);
    // auto part_info = PartInfo{std::move(part), py::set<node_t>()};
    auto begin = std::chrono::steady_clock::now();
    partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr>>(H, part);
    std::chrono::duration<double> last = std::chrono::steady_clock::now() - begin;
    std::cout << "time: " << last.count() << std::endl;
    CHECK(partMgr.totalcost >= 1104U);
    CHECK(partMgr.totalcost <= 5112U);
}

/*

Advantages:

1. Python-like, networkx
2. Check legalization, report
3. Generic
4. K buckets rather than K - 1
5. Time, space, and code complexity.
6. Design issues

*/
