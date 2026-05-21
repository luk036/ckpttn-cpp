#include <doctest/doctest.h>

#include <cstddef>
#include <map>
#include <netlistx/netlist.hpp>
#include <netlistx/readwrite.hpp>

using namespace std;

TEST_CASE("Test yosys sphere_netlist k-pin nets") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere_netlist.json");

    CHECK_EQ(hyprgraph.number_of_modules(), 65);
    CHECK_EQ(hyprgraph.number_of_nets(), 623);
    CHECK_EQ(hyprgraph.num_pads, 9);

    map<size_t, size_t> kpin_counts;
    for (const auto& net : hyprgraph.nets) {
        auto deg = hyprgraph.gr.degree(net);
        kpin_counts[deg]++;
    }

    CHECK_EQ(kpin_counts[1], 1);
    CHECK_EQ(kpin_counts[2], 467);
    CHECK_EQ(kpin_counts[3], 84);
    CHECK_EQ(kpin_counts[4], 66);
    CHECK_EQ(kpin_counts[13], 2);
    CHECK_EQ(kpin_counts[26], 3);
}

TEST_CASE("Test yosys sphere3hopf_netlist k-pin nets") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere3hopf_netlist.json");

    CHECK_EQ(hyprgraph.number_of_modules(), 188);
    CHECK_EQ(hyprgraph.number_of_nets(), 2825);
    CHECK_EQ(hyprgraph.num_pads, 8);

    map<size_t, size_t> kpin_counts;
    for (const auto& net : hyprgraph.nets) {
        auto deg = hyprgraph.gr.degree(net);
        kpin_counts[deg]++;
    }

    CHECK_EQ(kpin_counts[1], 28);
    CHECK_EQ(kpin_counts[2], 2314);
    CHECK_EQ(kpin_counts[3], 323);
    CHECK_EQ(kpin_counts[4], 1);
    CHECK_EQ(kpin_counts[5], 81);
    CHECK_EQ(kpin_counts[6], 17);
    CHECK_EQ(kpin_counts[7], 3);
    CHECK_EQ(kpin_counts[8], 1);
    CHECK_EQ(kpin_counts[9], 16);
    CHECK_EQ(kpin_counts[10], 1);
    CHECK_EQ(kpin_counts[11], 35);
    CHECK_EQ(kpin_counts[13], 1);
    CHECK_EQ(kpin_counts[17], 1);
    CHECK_EQ(kpin_counts[25], 1);
    CHECK_EQ(kpin_counts[32], 2);
}

TEST_CASE("Test yosys sphere3hopf_netlist_simple k-pin nets") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere3hopf_netlist_simple.json");

    CHECK_EQ(hyprgraph.number_of_modules(), 188);
    CHECK_EQ(hyprgraph.number_of_nets(), 2825);
    CHECK_EQ(hyprgraph.num_pads, 8);

    map<size_t, size_t> kpin_counts;
    for (const auto& net : hyprgraph.nets) {
        auto deg = hyprgraph.gr.degree(net);
        kpin_counts[deg]++;
    }

    CHECK_EQ(kpin_counts[1], 28);
    CHECK_EQ(kpin_counts[2], 2314);
    CHECK_EQ(kpin_counts[3], 323);
    CHECK_EQ(kpin_counts[4], 1);
    CHECK_EQ(kpin_counts[5], 81);
    CHECK_EQ(kpin_counts[6], 17);
    CHECK_EQ(kpin_counts[7], 3);
    CHECK_EQ(kpin_counts[8], 1);
    CHECK_EQ(kpin_counts[9], 16);
    CHECK_EQ(kpin_counts[10], 1);
    CHECK_EQ(kpin_counts[11], 35);
    CHECK_EQ(kpin_counts[13], 1);
    CHECK_EQ(kpin_counts[17], 1);
    CHECK_EQ(kpin_counts[25], 1);
    CHECK_EQ(kpin_counts[32], 2);
}
