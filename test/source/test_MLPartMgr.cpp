#include <doctest/doctest.h>  // for ResultBuilder, TestCase, CHECK

#include <chrono>                // for duration, operator-, steady_clock
#include <ckpttn/MLPartMgr.hpp>  // for MLPartMgr
// #include <experimental/random>
// #include <__config>     // for std
#include <cstdint>      // for uint8_t
#include <iostream>     // for operator<<, basic_ostream, endl, cout
#include <ratio>        // for ratio
#include <string_view>  // for string_view
#include <vector>       // for vector

#include "ckpttn/PartMgrBase.hpp"  // for SimpleNetlist
#include "ckpttn/netlist.hpp"      // for Netlist

template <typename Gnl> class FMBiConstrMgr;

class FMBiGainMgr;

template <typename Gnl> class FMKWayConstrMgr;

class FMKWayGainMgr;

template <typename GainMgr, typename ConstrMgr> class FMPartMgr;

using namespace std;

// using experimental::randint;

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& H, string_view areFileName);

TEST_CASE("Test MLBiPartMgr dwarf") {
    const auto H = create_dwarf();
    auto partMgr = MLPartMgr{0.3};
    auto part = vector<uint8_t>(H.number_of_modules(), 0);
    partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr<SimpleNetlist>>>(H, part);
    CHECK(partMgr.totalcost == 2U);
}

TEST_CASE("Test MLKWayPartMgr dwarf") {
    const auto H = create_dwarf();
    auto partMgr = MLPartMgr{0.4, 3};  // 0.3???
    auto part = vector<uint8_t>(H.number_of_modules(), 0);
    partMgr.run_FMPartition<FMPartMgr<FMKWayGainMgr, FMKWayConstrMgr<SimpleNetlist>>>(H, part);
    CHECK(partMgr.totalcost == 4U);
}

TEST_CASE("Test MLBiPartMgr p1") {
    const auto H = readNetD("../../testcases/p1.net");
    auto partMgr = MLPartMgr{0.3};
    partMgr.set_limitsize(500);

    auto mincost = 1000;
    for (auto i = 0; i != 10; ++i) {
        auto part = vector<uint8_t>(H.number_of_modules(), 0);
        auto whichPart = 0U;
        for (auto& elem : part) {
            whichPart ^= 1;
            elem = uint8_t(whichPart);
        }
        partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr<SimpleNetlist>>>(H, part);
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
        auto part = vector<uint8_t>(H.number_of_modules(), 0);
        auto whichPart = 0U;
        for (auto& elem : part) {
            whichPart ^= 1;
            elem = uint8_t(whichPart);
        }
        partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr<SimpleNetlist>>>(H, part);
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
    auto part = vector<uint8_t>(H.number_of_modules(), 0);
    // auto part_info = PartInfo{move(part), py::set<node_t>()};
    auto begin = chrono::steady_clock::now();
    partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr<SimpleNetlist>>>(H, part);
    chrono::duration<double> last = chrono::steady_clock::now() - begin;
    cout << "time: " << last.count() << endl;
    CHECK(partMgr.totalcost >= 1104U);
    CHECK(partMgr.totalcost <= 2041U);
}

TEST_CASE("Test MLBiPartMgr ibm18") {
    auto H = readNetD("../../testcases/ibm18.net");
    readAre(H, "../../testcases/ibm18.are");
    auto partMgr = MLPartMgr{0.45};
    partMgr.set_limitsize(24000);
    auto part = vector<uint8_t>(H.number_of_modules(), 0);
    // auto part_info = PartInfo{move(part), py::set<node_t>()};
    auto begin = chrono::steady_clock::now();
    partMgr.run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr<SimpleNetlist>>>(H, part);
    chrono::duration<double> last = chrono::steady_clock::now() - begin;
    cout << "time: " << last.count() << endl;
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
