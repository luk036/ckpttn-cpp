#include <ckpttn/MidLvlKWayPartMgr.hpp>
#include <cstdint>
#include <netlistx/netlist.hpp>
#include <vector>
#include <xnetwork/classes/graph.hpp>

#include "test_common.hpp"

TEST_CASE("Test MidLvlKWayPartMgr dwarf 3-way") {
    const auto hyprgraph = create_dwarf();
    const auto N = hyprgraph.number_of_modules();

    MidLvlKWayPartMgr part_mgr{0.4, 3};
    std::vector<std::uint8_t> part(N, 0);
    for (auto i = 0U; i < N; ++i) {
        part[i] = static_cast<std::uint8_t>(i % 3);
    }

    part_mgr.optimize(part, hyprgraph);
    CHECK_GE(part_mgr.total_cost, 0);
}
