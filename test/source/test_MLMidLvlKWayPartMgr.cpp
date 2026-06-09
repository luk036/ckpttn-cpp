#include <chrono>
#include <ckpttn/MLMidLvlKWayPartMgr.hpp>
#include <cstdint>
#include <iostream>
#include <netlistx/netlist.hpp>
#include <netlistx/readwrite.hpp>
#include <vector>
#include <xnetwork/classes/graph.hpp>

#include "test_common.hpp"

TEST_CASE("Test MLMidLvlKWayPartMgr dwarf 3-way") {
    const auto hyprgraph = create_dwarf();
    const auto N = hyprgraph.number_of_modules();

    MLMidLvlKWayPartMgr part_mgr{0.4, 3};
    std::vector<std::uint8_t> part(N, 0);
    for (auto i = 0U; i < N; ++i) {
        part[i] = static_cast<std::uint8_t>(i % 3);
    }

    part_mgr.optimize(part, hyprgraph);
    CHECK_GE(part_mgr.total_cost, 0);
}

TEST_CASE("Test MLMidLvlKWayPartMgr p1 3-way") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    const auto N = hyprgraph.number_of_modules();

    auto begin = std::chrono::steady_clock::now();
    MLMidLvlKWayPartMgr part_mgr{0.4, 3};
    std::vector<std::uint8_t> part(N, 0);
    for (auto i = 0U; i < N; ++i) {
        part[i] = static_cast<std::uint8_t>(i % 3);
    }

    part_mgr.optimize(part, hyprgraph);
    auto sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
    std::cout << "MLMidLvl KWay p1 3-way  time=" << sec << "s  cost=" << part_mgr.total_cost
              << "\n";
    CHECK_GE(part_mgr.total_cost, 0);
}
