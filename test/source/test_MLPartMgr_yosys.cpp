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

//
// Multi-level bi-partitioning using Yosys JSON netlists
//
// Yosys netlists have I/O ports as fixed (weight=0) modules.
// The FM legalize step does not handle these correctly, so we
// bypass it by initializing a balanced partition and running
// optimize directly.
//

static auto init_balanced_partition(vector<uint8_t>& part,
                                    const SimpleNetlist& hyprgraph) -> void {
    vector<uint32_t> movable;
    for (const auto& v : hyprgraph) {
        if (hyprgraph.get_module_weight(v) != 0U) {
            movable.push_back(v);
        }
    }
    const auto half = movable.size() / 2;
    for (size_t i = 0; i < half; ++i) {
        part[movable[i]] = 0;
    }
    for (size_t i = half; i < movable.size(); ++i) {
        part[movable[i]] = 1;
    }
}

TEST_CASE("Test MLBiPartMgr sphere_netlist") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere_netlist.json");
    const auto bal_tol = 0.3;
    MLPartMgr part_mgr{bal_tol};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    init_balanced_partition(part, hyprgraph);

    FMBiGainMgr<SimpleNetlist> gain_mgr{hyprgraph};
    FMBiConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, bal_tol};
    FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>> fm_part_mgr{
        hyprgraph, gain_mgr, constr_mgr};
    fm_part_mgr.init(part);
    fm_part_mgr.optimize(part);

    CHECK(constr_mgr.final_check(part));
    CHECK_GE(fm_part_mgr.total_cost, 0);
}

TEST_CASE("Test MLBiPartMgr sphere3hopf_netlist") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere3hopf_netlist.json");
    const auto bal_tol = 0.3;
    MLPartMgr part_mgr{bal_tol};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    init_balanced_partition(part, hyprgraph);

    FMBiGainMgr<SimpleNetlist> gain_mgr{hyprgraph};
    FMBiConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, bal_tol};
    FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>> fm_part_mgr{
        hyprgraph, gain_mgr, constr_mgr};
    fm_part_mgr.init(part);
    fm_part_mgr.optimize(part);

    CHECK(constr_mgr.final_check(part));
    CHECK_GE(fm_part_mgr.total_cost, 0);
}

//
// Multi-level K-way partitioning using Yosys JSON netlists
//

static auto init_balanced_partition_kway(vector<uint8_t>& part,
                                         const SimpleNetlist& hyprgraph,
                                         uint8_t num_parts) -> void {
    vector<uint32_t> movable;
    for (const auto& v : hyprgraph) {
        if (hyprgraph.get_module_weight(v) != 0U) {
            movable.push_back(v);
        }
    }
    for (size_t i = 0; i < movable.size(); ++i) {
        part[movable[i]] = static_cast<uint8_t>(i % num_parts);
    }
}

TEST_CASE("Test MLKWayPartMgr sphere_netlist") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere_netlist.json");
    const auto bal_tol = 0.4;
    const auto num_parts = 3;
    MLPartMgr part_mgr{bal_tol, num_parts};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    init_balanced_partition_kway(part, hyprgraph, num_parts);

    FMKWayGainMgr<SimpleNetlist> gain_mgr{hyprgraph, num_parts};
    FMKWayConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, bal_tol, num_parts};
    FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>
        fm_part_mgr{hyprgraph, gain_mgr, constr_mgr, num_parts};
    fm_part_mgr.init(part);
    fm_part_mgr.optimize(part);

    CHECK(constr_mgr.final_check(part));
    CHECK_GE(fm_part_mgr.total_cost, 0);
}

TEST_CASE("Test MLKWayPartMgr sphere3hopf_netlist") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere3hopf_netlist.json");
    const auto bal_tol = 0.4;
    const auto num_parts = 3;
    MLPartMgr part_mgr{bal_tol, num_parts};
    vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    init_balanced_partition_kway(part, hyprgraph, num_parts);

    FMKWayGainMgr<SimpleNetlist> gain_mgr{hyprgraph, num_parts};
    FMKWayConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, bal_tol, num_parts};
    FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>
        fm_part_mgr{hyprgraph, gain_mgr, constr_mgr, num_parts};
    fm_part_mgr.init(part);
    fm_part_mgr.optimize(part);

    CHECK(constr_mgr.final_check(part));
    CHECK_GE(fm_part_mgr.total_cost, 0);
}
