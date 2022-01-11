#pragma once

// **Special code for two-pin nets**
// Take a snapshot when a move make **negative** gain.
// Snapshot in the form of "interface"???
// #include "FMPartMgr.hpp" // import FMPartMgr
// #include "netlist.hpp"
#include <cassert>
#include <ckpttn/netlist.hpp>
#include <gsl/span>  // for span
#include <memory>    // std::unique_ptr
// #include <py2cpp/range.hpp>  // for range
// #include <ckpttn/FMConstrMgr.hpp>   // import LegalCheck

// forward declare
// template <typename nodeview_t, typename nodemap_t> struct Netlist;
// using RngIter = decltype(py::range(1));
// using SimpleNetlist = Netlist<RngIter, RngIter>;

// using node_t = typename SimpleNetlist::node_t;
// extern auto create_contraction_subgraph(const SimpleNetlist&, const py::set<node_t>&)
//     -> std::unique_ptr<SimpleHierNetlist>;

enum class LegalCheck;

/**
 * @brief Multilevel Partition Manager
 *
 */
class MLPartMgr {
  private:
    double BalTol;
    std::uint8_t K;
    size_t limitsize{7U};

  public:
    int totalcost{};

    /**
     * @brief Construct a new MLPartMgr object
     *
     * @param[in] BalTol
     */
    explicit MLPartMgr(double BalTol) : MLPartMgr(BalTol, 2) {}

    /**
     * @brief Construct a new MLPartMgr object
     *
     * @param[in] BalTol
     * @param[in] K
     */
    MLPartMgr(double BalTol, std::uint8_t K) : BalTol{BalTol}, K{K} {}

    void set_limitsize(size_t limit) { this->limitsize = limit; }

    /**
     * @brief run_Partition
     *
     * @tparam PartMgr
     * @param[in] H
     * @param[in,out] part
     * @return LegalCheck
     */
    template <typename PartMgr>
    auto run_FMPartition(const SimpleNetlist& H, gsl::span<std::uint8_t> part) -> LegalCheck;
};
