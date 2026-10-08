/**
 * @file MinhashProbe.cpp
 * @brief Time a single hypergraph contraction (where the MinHash pre-filter runs).
 *
 * Usage: bench_minhash_probe <netfile> <arefile|-> <reps>
 */

#include <chrono>
#include <ckpttn/HierNetlist.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <netlistx/netlist.hpp>
#include <py2cpp/set.hpp>
#include <string_view>

using node_t = SimpleNetlist::node_t;

extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& hyprgraph, std::string_view areFileName);
extern auto create_contracted_subgraph(const SimpleNetlist&, py::set<node_t>)
    -> std::unique_ptr<SimpleHierNetlist>;

int main(int argc, char** argv) {
    if (argc != 4) {
        std::fprintf(stderr, "usage: %s <netfile> <arefile|-> <reps>\n", argv[0]);
        return 2;
    }
    auto hyprgraph = readNetD(argv[1]);
    if (std::string_view{argv[2]} != "-") {
        readAre(hyprgraph, argv[2]);
    }
    const auto reps = std::atoi(argv[3]);

    for (int i = 0; i < reps; ++i) {
        const auto t0 = std::chrono::steady_clock::now();
        auto hgr2 = create_contracted_subgraph(hyprgraph, py::set<node_t>{});
        const auto t1 = std::chrono::steady_clock::now();
        const auto secs = std::chrono::duration<double>{t1 - t0}.count();
        std::printf("{\"modules_in\": %zu, \"modules_out\": %zu, \"time_s\": %.4f}\n",
                    static_cast<std::size_t>(hyprgraph.number_of_modules()),
                    static_cast<std::size_t>(hgr2->number_of_modules()), secs);
        std::fflush(stdout);
    }
    return 0;
}
