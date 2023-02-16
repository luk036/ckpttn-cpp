// -*- coding: utf-8 -*-
#include <doctest/doctest.h> // for ResultBuilder, CHECK, TestCase, TEST_CASE
// #include <__config>            // for std
#include <boost/utility/string_view.hpp> // for boost::string_view
#include <ckpttn/netlist.hpp>            // for Netlist, SimpleNetlist

using namespace std;

extern auto readNetD(boost::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hgr, boost::string_view areFileName);
extern void writeJSON(boost::string_view jsonFileName,
                      const SimpleNetlist &hgr);

TEST_CASE("Test Read Dwarf") {
  auto hgr = readNetD("../../testcases/dwarf1.netD");
  readAre(hgr, "../../testcases/dwarf1.are");

  CHECK(hgr.number_of_modules() == 7);
  CHECK(hgr.number_of_nets() == 5);
  // CHECK(hgr.number_of_pins() == 13);
  CHECK(hgr.get_max_degree() == 3);
  CHECK(hgr.get_max_net_degree() == 3);
  CHECK(!hgr.has_fixed_modules);
  CHECK(hgr.get_module_weight(1) == 2);
}

TEST_CASE("Test Read p1") {
  const auto hgr = readNetD("../../testcases/p1.net");

  CHECK(hgr.number_of_modules() == 833);
  CHECK(hgr.number_of_nets() == 902);
  // CHECK(hgr.number_of_pins() == 2908);
  CHECK(hgr.get_max_degree() == 9);
  CHECK(hgr.get_max_net_degree() == 18);
  CHECK(!hgr.has_fixed_modules);
  CHECK(hgr.get_module_weight(1) == 1);
}

TEST_CASE("Test Read ibm01") {
  const auto hgr = readNetD("../../testcases/ibm01.net");

  CHECK(hgr.number_of_modules() == 12752);
  CHECK(hgr.number_of_nets() == 14111);
  // CHECK(hgr.number_of_pins() == 2908);
  CHECK(hgr.get_max_degree() == 39);
  CHECK(hgr.get_max_net_degree() == 42);
  CHECK(!hgr.has_fixed_modules);
  CHECK(hgr.get_module_weight(1) == 1);
}

TEST_CASE("Test Read ibm18") {
  const auto hgr = readNetD("../../testcases/ibm18.net");

  CHECK(hgr.number_of_modules() == 210613);
  CHECK(hgr.number_of_nets() == 201920);
  // CHECK(hgr.number_of_pins() == 2908);
  CHECK(hgr.get_max_degree() == 97);
  CHECK(hgr.get_max_net_degree() == 66);
  CHECK(!hgr.has_fixed_modules);
  CHECK(hgr.get_module_weight(1) == 1);
}

TEST_CASE("Test Write Dwarf") {
  auto hgr = readNetD("../../testcases/dwarf1.netD");
  readAre(hgr, "../../testcases/dwarf1.are");
  writeJSON("../../testcases/dwarf1.json", hgr);

  CHECK(hgr.number_of_modules() == 7);
  CHECK(hgr.number_of_nets() == 5);
  // CHECK(hgr.number_of_pins() == 13);
}

TEST_CASE("Test Write p1") {
  const auto hgr = readNetD("../../testcases/p1.net");
  writeJSON("../../testcases/p1.json", hgr);
}
