#pragma once

#include <doctest/doctest.h>

#include <ckpttn/FMConstrMgr.hpp>  // for FMConstrMgr, LegalCheck, move_info_v
#include <ckpttn/NNPartMgr.hpp>    // for NNPartMgr
#include <cstdint>                 // for uint8_t
#include <netlistx/netlist.hpp>    // for SimpleNetlist
#include <string_view>             // for std::string_view
#include <vector>                  // for vector

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& hyprgraph, std::string_view areFileName);

template <typename GainMgr, typename ConstrMgr> void run_NNPartMgr(const SimpleNetlist& hyprgraph) {
    GainMgr gain_mgr{hyprgraph};
    ConstrMgr constr_mgr{hyprgraph, 0.4};
    NNPartMgr<SimpleNetlist, GainMgr, ConstrMgr> part_mgr{hyprgraph, gain_mgr, constr_mgr, 2};
    std::vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    auto legal_check = part_mgr.legalize(part);
    CHECK_EQ(legal_check, LegalCheck::AllSatisfied);
    auto totalcostbefore = part_mgr.total_cost;
    part_mgr.optimize(part);
    CHECK_GE(totalcostbefore, 0);
    CHECK_LE(part_mgr.total_cost, totalcostbefore);
    CHECK_GE(part_mgr.total_cost, 0);
    totalcostbefore = part_mgr.total_cost;
    part_mgr.init(part);
    CHECK_EQ(part_mgr.total_cost, totalcostbefore);
}

template <typename GainMgr, typename ConstrMgr>
void run_NNPartMgr(const SimpleNetlist& hyprgraph, uint8_t num_parts) {
    GainMgr gain_mgr{hyprgraph, num_parts};
    ConstrMgr constr_mgr{hyprgraph, 0.4, num_parts};
    NNPartMgr<SimpleNetlist, GainMgr, ConstrMgr> part_mgr{hyprgraph, gain_mgr, constr_mgr,
                                                          num_parts};
    std::vector<uint8_t> part(hyprgraph.number_of_modules(), 0);

    auto legal_check = part_mgr.legalize(part);
    CHECK_EQ(legal_check, LegalCheck::AllSatisfied);
    auto totalcostbefore = part_mgr.total_cost;
    part_mgr.optimize(part);
    CHECK_GE(totalcostbefore, 0);
    CHECK_LE(part_mgr.total_cost, totalcostbefore);
    CHECK_GE(part_mgr.total_cost, 0);
    totalcostbefore = part_mgr.total_cost;
    part_mgr.init(part);
    CHECK_EQ(part_mgr.total_cost, totalcostbefore);
}
