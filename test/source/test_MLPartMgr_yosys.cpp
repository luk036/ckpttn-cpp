#include <doctest/doctest.h>

#include <algorithm>
#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMKWayConstrMgr.hpp>
#include <ckpttn/FMKWayGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/MLPartMgr.hpp>
#include <cstdint>
#include <netlistx/netlist.hpp>
#include <netlistx/readwrite.hpp>
#include <vector>

using namespace std;

TEST_CASE("Test MLBiPartMgr sphere_netlist") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere_netlist.json");
    const auto bal_tol = 0.3;
    MLPartMgr part_mgr{bal_tol};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    auto legal_check = part_mgr.run_FMPartition<
        SimpleNetlist,
        FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>>(
        hyprgraph, part);
    CHECK_EQ(legal_check, LegalCheck::AllSatisfied);

    auto constr_mgr = FMBiConstrMgr<SimpleNetlist>(hyprgraph, bal_tol);
    CHECK(constr_mgr.final_check(part));
    CHECK_GE(part_mgr.total_cost, 0);
}

TEST_CASE("Test MLBiPartMgr sphere3hopf_netlist") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere3hopf_netlist.json");
    const auto bal_tol = 0.3;
    MLPartMgr part_mgr{bal_tol};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    auto legal_check = part_mgr.run_FMPartition<
        SimpleNetlist,
        FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>>(
        hyprgraph, part);
    CHECK_EQ(legal_check, LegalCheck::AllSatisfied);

    auto constr_mgr = FMBiConstrMgr<SimpleNetlist>(hyprgraph, bal_tol);
    CHECK(constr_mgr.final_check(part));
    CHECK_GE(part_mgr.total_cost, 0);
}

TEST_CASE("Test MLKWayPartMgr sphere_netlist") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere_netlist.json");
    const auto bal_tol = 0.4;
    const auto num_parts = 3;
    MLPartMgr part_mgr{bal_tol, num_parts};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    auto legal_check = part_mgr.run_FMPartition<
        SimpleNetlist,
        FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>>(
        hyprgraph, part);
    CHECK_EQ(legal_check, LegalCheck::AllSatisfied);

    auto constr_mgr = FMKWayConstrMgr<SimpleNetlist>(hyprgraph, bal_tol, num_parts);
    CHECK(constr_mgr.final_check(part));
    CHECK_GE(part_mgr.total_cost, 0);
}

TEST_CASE("Test MLKWayPartMgr sphere3hopf_netlist") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere3hopf_netlist.json");
    const auto bal_tol = 0.4;
    const auto num_parts = 3;
    MLPartMgr part_mgr{bal_tol, num_parts};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    auto legal_check = part_mgr.run_FMPartition<
        SimpleNetlist,
        FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>>(
        hyprgraph, part);
    CHECK_EQ(legal_check, LegalCheck::AllSatisfied);

    auto constr_mgr = FMKWayConstrMgr<SimpleNetlist>(hyprgraph, bal_tol, num_parts);
    CHECK(constr_mgr.final_check(part));
    CHECK_GE(part_mgr.total_cost, 0);
}
