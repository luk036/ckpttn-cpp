// -*- coding: utf-8 -*-
#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase, TEST_CASE

#include <__config>            // for std
#include <ckpttn/netlist.hpp>  // for Netlist, SimpleNetlist
#include <string_view>         // for string_view

using namespace std;

extern auto readNetD(string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& H, string_view areFileName);
extern void writeJSON(string_view jsonFileName, const SimpleNetlist& H);

TEST_CASE("Test Read Dwarf") {
    auto H = readNetD("../../testcases/dwarf1.netD");
    readAre(H, "../../testcases/dwarf1.are");

    CHECK(H.number_of_modules() == 7);
    CHECK(H.number_of_nets() == 5);
    // CHECK(H.number_of_pins() == 13);
    CHECK(H.get_max_degree() == 3);
    CHECK(H.get_max_net_degree() == 3);
    CHECK(!H.has_fixed_modules);
    CHECK(H.get_module_weight(1) == 2);
}

TEST_CASE("Test Read p1") {
    const auto H = readNetD("../../testcases/p1.net");

    CHECK(H.number_of_modules() == 833);
    CHECK(H.number_of_nets() == 902);
    // CHECK(H.number_of_pins() == 2908);
    CHECK(H.get_max_degree() == 9);
    CHECK(H.get_max_net_degree() == 18);
    CHECK(!H.has_fixed_modules);
    CHECK(H.get_module_weight(1) == 1);
}

TEST_CASE("Test Read ibm01") {
    const auto H = readNetD("../../testcases/ibm01.net");

    CHECK(H.number_of_modules() == 12752);
    CHECK(H.number_of_nets() == 14111);
    // CHECK(H.number_of_pins() == 2908);
    CHECK(H.get_max_degree() == 39);
    CHECK(H.get_max_net_degree() == 42);
    CHECK(!H.has_fixed_modules);
    CHECK(H.get_module_weight(1) == 1);
}

TEST_CASE("Test Read ibm18") {
    const auto H = readNetD("../../testcases/ibm18.net");

    CHECK(H.number_of_modules() == 210613);
    CHECK(H.number_of_nets() == 201920);
    // CHECK(H.number_of_pins() == 2908);
    CHECK(H.get_max_degree() == 97);
    CHECK(H.get_max_net_degree() == 66);
    CHECK(!H.has_fixed_modules);
    CHECK(H.get_module_weight(1) == 1);
}

TEST_CASE("Test Write Dwarf") {
    auto H = readNetD("../../testcases/dwarf1.netD");
    readAre(H, "../../testcases/dwarf1.are");
    writeJSON("../../testcases/dwarf1.json", H);

    CHECK(H.number_of_modules() == 7);
    CHECK(H.number_of_nets() == 5);
    // CHECK(H.number_of_pins() == 13);
}

TEST_CASE("Test Write p1") {
    const auto H = readNetD("../../testcases/p1.net");
    writeJSON("../../testcases/p1.json", H);
}
