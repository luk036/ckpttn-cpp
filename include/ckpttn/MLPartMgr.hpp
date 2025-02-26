#pragma once

// **Special code for two-pin nets**
// Take a snapshot when a move make **negative** gain.
// Snapshot in the form of "interface"???
// #include "FMPartMgr.hpp" // import FMPartMgr
// #include "netlist.hpp"
#include <cassert>
#include <memory>  // std::unique_ptr
#include <span>    // for span
// #include <py2cpp/range.hpp>  // for range
// #include <ckpttn/FMConstrMgr.hpp>   // import LegalCheck

// forward declare
// template <typename nodeview_t, typename nodemap_t> struct Netlist;
// using RngIter = decltype(py::range(1));
// using SimpleNetlist = Netlist<RngIter, RngIter>;

// using node_t = typename SimpleNetlist::node_t;
// extern auto create_contracted_subgraph(const SimpleNetlist&, const
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
     * @brief Constructs a new MLPartMgr object with the given balance tolerance and a default
     * number of partitions (2).
     *
     * @param[in] bal_tol The balance tolerance for the partitioning.
     */
    explicit MLPartMgr(double bal_tol) : MLPartMgr(bal_tol, 2) {}

    /**
     * @brief Constructs a new MLPartMgr object with the given balance tolerance and number of
     * partitions.
     *
     * @param[in] bal_tol The balance tolerance for the partitioning.
     * @param[in] num_parts The number of partitions to create.
     */
    MLPartMgr(double bal_tol, std::uint8_t num_parts) : bal_tol{bal_tol}, num_parts{num_parts} {}

    /**
     * @brief Sets the limit size for the partitioning.
     *
     * @param[in] limit The new limit size for the partitioning.
     */
    void set_limitsize(size_t limit) { this->limitsize = limit; }

    /**
     * @brief Runs the Fiduccia-Mattheyses (FM) partitioning algorithm on the given hypergraph.
     *
     * @tparam Gnl The type of the hypergraph.
     * @tparam PartMgr The type of the partition manager.
     * @param[in] hyprgraph The input hypergraph to partition.
     * @param[in,out] part The partition vector to store the partitioning results.
     * @return LegalCheck The legality check result of the partitioning.
     */
    template <typename Gnl, typename PartMgr>
    auto run_FMPartition(const Gnl &hyprgraph, std::span<std::uint8_t> part) -> LegalCheck;
};
