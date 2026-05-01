#include <ckpttn/readwrite.hpp>
#include <netlistx/netlist.hpp>
#include <sstream>

#include "test_common.hpp"

TEST_CASE("detect_input_format") {
    CHECK_EQ(detect_input_format("test.hgr"), InputFormat::hmetis);
    CHECK_EQ(detect_input_format("test.graph"), InputFormat::hmetis);
    CHECK_EQ(detect_input_format("test.json"), InputFormat::json);
    CHECK_EQ(detect_input_format("test.net"), InputFormat::netD);
    CHECK_EQ(detect_input_format("test.dimacs"), InputFormat::dimacs);
    CHECK_EQ(detect_input_format("unknown"), InputFormat::auto_detect);
}

TEST_CASE("read_netD_format") {
    auto hyprgraph = readNetD("../../testcases/p1.net");
    CHECK_GT(hyprgraph.number_of_modules(), 0);
    CHECK_GT(hyprgraph.number_of_nets(), 0);
}

// TEST_CASE("read_hmetis_format") {
//     auto hyprgraph = readNetD("../../testcases/test.hgr");
//     CHECK_EQ(hyprgraph.number_of_modules(), 5);
//     CHECK_EQ(hyprgraph.number_of_nets(), 4);
// }

TEST_CASE("write_hmetis_partition") {
    std::vector<std::uint8_t> part = {0, 1, 0, 1, 0};
    std::ostringstream oss;
    write_hmetis_partition(part, oss);
    CHECK_EQ(oss.str(), "0\n1\n0\n1\n0\n");
}

TEST_CASE("write_json_partition") {
    std::vector<std::uint8_t> part = {0, 1, 0, 1, 0};
    std::ostringstream oss;
    write_json_partition(part, oss);
    CHECK_EQ(oss.str(), "[0, 1, 0, 1, 0]\n");
}

TEST_CASE("write_partition") {
    std::vector<std::uint8_t> part = {0, 1, 0, 1, 0};

    std::ostringstream oss1;
    write_partition(part, oss1, OutputFormat::hmetis);
    CHECK_EQ(oss1.str(), "0\n1\n0\n1\n0\n");

    std::ostringstream oss2;
    write_partition(part, oss2, OutputFormat::json);
    CHECK_EQ(oss2.str(), "[0, 1, 0, 1, 0]\n");
}

TEST_CASE("read_hypergraph netD") {
    auto hyprgraph = readNetD("../../testcases/p1.net");
    CHECK_GT(hyprgraph.number_of_modules(), 0);
}

TEST_CASE("read_hypergraph auto") {
    auto hyprgraph = readNetD("../../testcases/p1.net");
    CHECK_GT(hyprgraph.number_of_modules(), 0);
}