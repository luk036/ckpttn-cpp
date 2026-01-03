#include <doctest/doctest.h>  // for ResultBuilder, TestCase, CHECK

#include <chrono>                // for duration, operator-, steady_clock
#include <ckpttn/MLPartMgr.hpp>  // for MLPartMgr
#include <cstdint>               // for uint8_t
#include <iostream>              // for operator<<, basic_ostream, endl, cout
#include <netlistx/netlist.hpp>  // for Netlist
#include <ratio>                 // for ratio
#include <string_view>           // for std::string_view
#include <vector>                // for vector

#include "ckpttn/PartMgrBase.hpp"  // for SimpleNetlist

template <typename Gnl> class FMBiConstrMgr;
template <typename Gnl> class FMBiGainMgr;
template <typename Gnl> class FMKWayConstrMgr;
template <typename Gnl> class FMKWayGainMgr;
template <typename Gnl, typename GainMgr, typename ConstrMgr> class FMPartMgr;

using namespace std;

// using experimental::randint;

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hyprgraph, std::string_view areFileName);

TEST_CASE("Test MLBiPartMgr dwarf") {
    const auto hyprgraph = create_dwarf();
    MLPartMgr part_mgr{0.3};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    part_mgr.run_FMPartition<SimpleNetlist, FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>,
                                                      FMBiConstrMgr<SimpleNetlist>>>(hyprgraph,
                                                                                     part);
    CHECK(part_mgr.total_cost == 2U);
}

TEST_CASE("Test MLKWayPartMgr dwarf") {
    const auto hyprgraph = create_dwarf();
    MLPartMgr part_mgr{0.4, 3};  // 0.3???
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    part_mgr.run_FMPartition<SimpleNetlist, FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>,
                                                      FMKWayConstrMgr<SimpleNetlist>>>(hyprgraph,
                                                                                       part);
    CHECK(part_mgr.total_cost == 4U);
}

TEST_CASE("Test MLBiPartMgr p1") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    MLPartMgr part_mgr{0.3};
    part_mgr.set_limitsize(500);

    auto mincost = 1000;
    for (auto idx = 0; idx != 10; ++idx) {
        auto part = vector<uint8_t>(hyprgraph.number_of_modules(), 0);
        auto whichPart = uint8_t(0);
        for (auto &elem : part) {
            whichPart ^= 1;
            elem = whichPart;
        }
        part_mgr.run_FMPartition<SimpleNetlist, FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>,
                                                          FMBiConstrMgr<SimpleNetlist>>>(hyprgraph,
                                                                                         part);
        if (mincost > part_mgr.total_cost) {
            mincost = part_mgr.total_cost;
        }
    }
    // CHECK(part_mgr.total_cost >= 50);
    // CHECK(part_mgr.total_cost <= 50);
    CHECK(mincost >= 29U);
    CHECK(mincost <= 73U);
}

TEST_CASE("Test MLBiPartMgr ibm01") {
    auto hyprgraph = readNetD("../../testcases/ibm01.net");
    readAre(hyprgraph, "../../testcases/ibm01.are");
    MLPartMgr part_mgr{0.4};
    part_mgr.set_limitsize(400);

    auto mincost = 1000;
    for (auto idx = 0; idx != 10; ++idx) {
        auto part = vector<uint8_t>(hyprgraph.number_of_modules(), 0);
        auto whichPart = uint8_t(0);
        for (auto &elem : part) {
            whichPart ^= 1;
            elem = whichPart;
        }
        part_mgr.run_FMPartition<SimpleNetlist, FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>,
                                                          FMBiConstrMgr<SimpleNetlist>>>(hyprgraph,
                                                                                         part);
        if (mincost > part_mgr.total_cost) {
            mincost = part_mgr.total_cost;
        }
    }
    // CHECK(part_mgr.total_cost >= 650);
    // CHECK(part_mgr.total_cost <= 650);
    CHECK(mincost >= 221U);
    CHECK(mincost <= 695U);
}

TEST_CASE("Test MLBiPartMgr ibm03") {
    auto hyprgraph = readNetD("../../testcases/ibm03.net");
    readAre(hyprgraph, "../../testcases/ibm03.are");
    MLPartMgr part_mgr{0.45};
    part_mgr.set_limitsize(300);
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    // auto part_info = PartInfo{move(part), py::set<node_t>()};
    auto begin = chrono::steady_clock::now();
    part_mgr.run_FMPartition<SimpleNetlist, FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>,
                                                      FMBiConstrMgr<SimpleNetlist>>>(hyprgraph,
                                                                                     part);
    chrono::duration<double> last = chrono::steady_clock::now() - begin;
    cout << "time: " << last.count() << endl;
    CHECK(part_mgr.total_cost >= 1104U);
    CHECK(part_mgr.total_cost <= 2041U);
}

TEST_CASE("Test MLBiPartMgr ibm18") {
    auto hyprgraph = readNetD("../../testcases/ibm18.net");
    readAre(hyprgraph, "../../testcases/ibm18.are");
    MLPartMgr part_mgr{0.45};
    part_mgr.set_limitsize(24000);
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    // auto part_info = PartInfo{move(part), py::set<node_t>()};
    auto begin = chrono::steady_clock::now();
    part_mgr.run_FMPartition<SimpleNetlist, FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>,
                                                      FMBiConstrMgr<SimpleNetlist>>>(hyprgraph,
                                                                                     part);
    chrono::duration<double> last = chrono::steady_clock::now() - begin;
    cout << "time: " << last.count() << endl;
    CHECK(part_mgr.total_cost >= 1104U);
    CHECK(part_mgr.total_cost <= 5112U);
}

/*

Advantages:

1. Python-like, networkx
2. Check legalization, report
3. Generic
4. num_parts buckets rather than num_parts - 1
5. Time, space, and code complexity.
6. Design issues

*/
