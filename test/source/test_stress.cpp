#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/MLPartMgr.hpp>
#include <iostream>

#include "test_common.hpp"

TEST_CASE("Stress Test MLBiPartMgr ibm18") {
    auto hyprgraph = readNetD("../../testcases/ibm18.net");
    readAre(hyprgraph, "../../testcases/ibm18.are");

    MLPartMgr part_mgr{0.45};
    part_mgr.set_limitsize(24000);

    const int num_iterations = 2;
    unsigned int total_cost_sum = 0;

    for (int i = 0; i < num_iterations; ++i) {
        std::vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
        part_mgr.run_FMPartition<SimpleNetlist, FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>,
                                                          FMBiConstrMgr<SimpleNetlist>>>(hyprgraph,
                                                                                         part);
        total_cost_sum += part_mgr.total_cost;
    }

    double average_cost = static_cast<double>(total_cost_sum) / num_iterations;
    std::cout << "Average cost over " << num_iterations << " iterations: " << average_cost
              << std::endl;

    CHECK(average_cost > 0);
}
