#pragma once

#include <ckpttn/FMPartMgr.hpp>      // for FMPartMgr
#include <cstdint>                   // for uint8_t
#include <netlistx/netlist.hpp>      // for SimpleNetlist
#include <string_view>               // for std::string_view
#include <vector>                    // for vector
#include <doctest/doctest.h>

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hyprgraph, std::string_view areFileName);

template <typename GainMgr, typename ConstrMgr>
void run_PartMgr(const SimpleNetlist &hyprgraph) {
    GainMgr gain_mgr{hyprgraph};
    ConstrMgr constr_mgr{hyprgraph, 0.4};
    FMPartMgr<SimpleNetlist, GainMgr, ConstrMgr> part_mgr{hyprgraph, gain_mgr, constr_mgr};
    std::vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
    part_mgr.legalize(part);
    auto totalcostbefore = part_mgr.total_cost;
    part_mgr.optimize(part);
    CHECK(totalcostbefore >= 0);
    CHECK(part_mgr.total_cost <= totalcostbefore);
    CHECK(part_mgr.total_cost >= 0);
    totalcostbefore = part_mgr.total_cost;
    part_mgr.init(part);
    CHECK(part_mgr.total_cost == totalcostbefore);
}

template <typename GainMgr, typename ConstrMgr>
void run_PartMgr(const SimpleNetlist &hyprgraph, uint8_t num_parts) {
    GainMgr gain_mgr{hyprgraph, num_parts};
    ConstrMgr constr_mgr{hyprgraph, 0.4, num_parts};
    FMPartMgr<SimpleNetlist, GainMgr, ConstrMgr> part_mgr{hyprgraph, gain_mgr, constr_mgr, num_parts};
    std::vector<uint8_t> part(hyprgraph.number_of_modules(), 0);

    part_mgr.legalize(part);
    auto totalcostbefore = part_mgr.total_cost;
    part_mgr.optimize(part);
    CHECK(totalcostbefore >= 0);
    CHECK(part_mgr.total_cost <= totalcostbefore);
    CHECK(part_mgr.total_cost >= 0);
    totalcostbefore = part_mgr.total_cost;
    part_mgr.init(part);
    CHECK(part_mgr.total_cost == totalcostbefore);
}
