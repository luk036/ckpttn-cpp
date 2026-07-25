#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/MLPartMgr.hpp>
#include <cstdint>
#include <iostream>
#include <netlistx/netlist.hpp>
#include <string_view>
#include <vector>

extern auto readNetD(std::string_view) -> SimpleNetlist;
extern void readAre(SimpleNetlist&, std::string_view);

int main() {
    auto hyprgraph = readNetD("../../testcases/ibm18.net");
    readAre(hyprgraph, "../../testcases/ibm18.are");

    ankerl::nanobench::Bench bench;
    bench.title("Stress MLBiPartMgr ibm18").unit("op").warmup(1).epochs(2);

    bench.run("MLBiPartMgr ibm18", [&] {
        MLPartMgr part_mgr{0.45};
        part_mgr.set_limitsize(24000);
        std::vector<uint8_t> part(hyprgraph.number_of_modules(), 0);
        part_mgr.run_Partition<SimpleNetlist,
                               FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>,
                                         FMBiConstrMgr<SimpleNetlist>>>(hyprgraph, part);
        ankerl::nanobench::doNotOptimizeAway(part);
    });
}
