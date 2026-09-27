/**
 * @file NNPartMgr.hpp
 * @brief No-Nonsense partitioning algorithm manager
 */

#pragma once

#include <cstddef>  // for size_t
#include <cstdint>  // for uint8_t
#include <span>     // for span

#include "PartMgrBase.hpp"  // for PartMgrBase

// forward declare
// template <typename graph_t> struct Netlist;
// using SimpleNetlist = Netlist<xnetwork::SimpleGraph>;

/**
 * @brief No-Nonsense Partitioning Algorithm Manager
 *
 * `NNPartMgr` reuses the shared FM algorithm skeleton from `PartMgrBase`
 * (Template Method pattern: `init`/`legalize`/`optimize`) and overrides
 * only the single-pass behaviour: unlike FM it is a pure greedy local search
 * that stops at the first non-positive-gain move (no look-ahead / snapshot /
 * rollback) and never locks a moved vertex. The gain-computation and
 * constraint-validation strategies remain injected as template parameters.
 *
 * @tparam Gnl
 * @tparam GainMgr
 * @tparam ConstrMgr
 */
template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
class NNPartMgr : public PartMgrBase<Gnl, GainMgr, ConstrMgr> {
    using Base = PartMgrBase<Gnl, GainMgr, ConstrMgr>;

  public:
    /**
     * @brief Constructs a new NNPartMgr object
     *
     * @param[in] hyprgraph The hypergraph to be partitioned
     * @param[in,out] gain_mgr The gain manager used for the partitioning
     * @param[in,out] constr_mgr The constraint manager used for the partitioning
     * @param[in] num_parts The number of partitions to create
     */
    NNPartMgr(const Gnl& hyprgraph, GainMgr& gain_mgr, ConstrMgr& constr_mgr, size_t num_parts)
        : Base{hyprgraph, gain_mgr, constr_mgr, num_parts} {}

  protected:
    /**
     * @brief Performs a single pass of the No-Nonsense optimization algorithm.
     *
     * Pure greedy local search: repeatedly takes the highest-gain move from the
     * gain buckets while its gain is strictly positive, and stops at the first
     * non-positive-gain move. It performs no snapshotting/rollback and does not
     * lock moved vertices.
     *
     * @param[in,out] part The partition vector to optimize
     */
    void _optimize_1pass(std::span<std::uint8_t> part) override;
};
