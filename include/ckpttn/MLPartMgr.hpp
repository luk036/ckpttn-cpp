#pragma once

// **Special code for two-pin nets**
// Take a snapshot when a move make **negative** gain.
// Snapshot in the form of "interface"???
// #include "FMPartMgr.hpp" // import FMPartMgr
// #include "netlist.hpp"
#include <cassert>
#include <gsl/span> // for span
#include <memory>   // std::unique_ptr
// #include <py2cpp/range.hpp>  // for range
// #include <ckpttn/FMConstrMgr.hpp>   // import LegalCheck

// forward declare
// template <typename nodeview_t, typename nodemap_t> struct Netlist;
// using RngIter = decltype(py::range(1));
// using SimpleNetlist = Netlist<RngIter, RngIter>;

// using node_t = typename SimpleNetlist::node_t;
// extern auto create_contraction_subgraph(const SimpleNetlist&, const
// py::set<node_t>&)
//     -> std::unique_ptr<SimpleHierNetlist>;

enum class LegalCheck;

/**
 * @brief Multilevel Partition Manager
 *
 */
class MLPartMgr {
private:
  double bal_tol;
  std::uint8_t num_parts;
  size_t limitsize{7U};

public:
  int total_cost{};

  /**
   * @brief Construct a new MLPartMgr object
   *
   * @param[in] bal_tol
   */
  explicit MLPartMgr(double bal_tol) : MLPartMgr(bal_tol, 2) {}

  /**
   * @brief Construct a new MLPartMgr object
   *
   * @param[in] bal_tol
   * @param[in] num_parts
   */
  MLPartMgr(double bal_tol, std::uint8_t num_parts)
      : bal_tol{bal_tol}, num_parts{num_parts} {}

  void set_limitsize(size_t limit) { this->limitsize = limit; }

  /**
   * @brief run_Partition
   *
   * @tparam Gnl
   * @tparam PartMgr
   * @param[in] hgr
   * @param[in,out] part
   * @return LegalCheck
   */
  template <typename Gnl, typename PartMgr>
  auto run_FMPartition(const Gnl &hgr, gsl::span<std::uint8_t> part)
      -> LegalCheck;
};
