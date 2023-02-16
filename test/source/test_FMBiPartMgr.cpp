#include <doctest/doctest.h> // for ResultBuilder, TestCase, CHECK
// #include <__config>                  // for std
#include <boost/utility/string_view.hpp> // for boost::string_view
#include <ckpttn/FMBiConstrMgr.hpp>      // for FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>        // for FMBiGainMgr
#include <ckpttn/FMPartMgr.hpp>          // for FMPartMgr
#include <cstdint>                       // for uint8_t
#include <vector>                        // for vector

#include "ckpttn/netlist.hpp" // for SimpleNetlist

using namespace std;

extern auto create_test_netlist()
    -> SimpleNetlist;                        // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist; // import create_dwarf
extern auto readNetD(boost::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hgr, boost::string_view areFileName);

/**
 * @brief Run test cases
 *
 * @param[in] hgr
 */
void run_FMBiPartMgr(const SimpleNetlist &hgr) {
  FMBiGainMgr<SimpleNetlist> gain_mgr{hgr};
  FMBiConstrMgr<SimpleNetlist> constr_mgr{hgr, 0.4};
  FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>,
            FMBiConstrMgr<SimpleNetlist>>
      part_mgr{hgr, gain_mgr, constr_mgr};
  vector<uint8_t> part(hgr.number_of_modules(), 0);
  part_mgr.legalize(part);
  auto totalcostbefore = part_mgr.totalcost;
  part_mgr.optimize(part);
  CHECK(totalcostbefore >= 0);
  CHECK(part_mgr.totalcost <= totalcostbefore);
  CHECK(part_mgr.totalcost >= 0);
  totalcostbefore = part_mgr.totalcost;
  part_mgr.init(part);
  CHECK(part_mgr.totalcost == totalcostbefore);
}

TEST_CASE("Test FMBiPartMgr") {
  const auto hgr = create_test_netlist();
  run_FMBiPartMgr(hgr);
}

TEST_CASE("Test FMBiPartMgr dwarf") {
  const auto hgr = create_dwarf();
  run_FMBiPartMgr(hgr);
}

TEST_CASE("Test FMBiPartMgr p1") {
  const auto hgr = readNetD("../../testcases/p1.net");
  run_FMBiPartMgr(hgr);
}

// TEST_CASE("Test FMBiPartMgr ibm01")
// {
//     auto hgr = readNetD("../../testcases/ibm01.net");
//     readAre(hgr, "../../testcases/ibm01.are");
//     run_FMBiPartMgr(hgr);
// }

// TEST_CASE("Test FMBiPartMgr ibm18")
// {
//     auto hgr = readNetD("../../testcases/ibm18.net");
//     readAre(hgr, "../../testcases/ibm18.are");
//     run_FMBiPartMgr(hgr);
// }
