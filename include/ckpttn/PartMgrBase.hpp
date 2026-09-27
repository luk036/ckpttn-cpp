/**
 * @file PartMgrBase.hpp
 * @brief Base class for Fiduccia-Mattheyses Partitioning Algorithm Manager
 */

#pragma once

// **Special code for two-pin nets**
// Take a snapshot when a move make **negative** gain.
// Snapshot in the form of "interface"???

#include <cstdint>  // for uint8_t
#include <span>     // for span
#include <vector>   // for vector
// #include <xnetwork/classes/graph.hpp>

#include "LegalCheck.hpp"  // for LegalCheck

// forward declare
// template <typename graph_t> struct Netlist;
// using SimpleNetlist = Netlist<xnetwork::SimpleGraph>;

/**
 * @brief Fiduccia-Mattheyses Partitioning Algorithm Manager Base
 *
 * Implements the **Template Method pattern**: `PartMgrBase` defines the
 * algorithm skeleton (`optimize` → `_optimize_1pass`) while the actual
 * move-selection steps are overridden by derived manager classes. This is
 * combined with **Strategy** via template policy (policy-based design): the
 * `GainMgr` and `ConstrMgr` are injected as template parameters, so the
 * gain-computation and constraint-validation strategies can be swapped at
 * compile time.
 *
 * `PartMgrBase` is a base class for managing the Fiduccia-Mattheyses
 * Partitioning Algorithm. It takes three template parameters: `Gnl` (graph
 * type), `GainMgr` (gain manager type), and `ConstrMgr` (constraint manager
 * type).
 *
 * In this partitioning method, the next solution $s'$ considered after
 * solution $s$ is dervied by first applying a sequence of
 * $t$ changes (moves) to $s$ (with $t$ dependent from
 * $s$ and from the specific heuristic method), thus obtaining a
 * sequence of solution $s,...,s_t$ and by successively
 * choosing the best among these solutions.
 *
 * In order to do that, heuristics refer to a measure of the gain (and
 * balance condition) associated to any sequence of changes performed on
 * the current solution. Moreover, the length of the sequence generated
 * is determined by evaluting a suitably defined $stopping rule$ at
 * each iteration.
 *
 * Reference:
 *   gr. Ausiello et al., Complexity and Approximation: Combinatorial
 * Optimization Problems and Their Approximability Properties, Section 10.3.2.
 *
 * @note Design patterns: **Template Method** (algorithm skeleton in base,
 * move-selection steps overridden by derived classes) combined with
 * **Strategy** via template policy (policy-based design: `GainMgr`/`ConstrMgr`
 * injected as template parameters; realized by the `gain_mgr` and `validator`
 * members).
 *
 * @tparam Gnl
 * @tparam GainMgr
 * @tparam ConstrMgr
 * @tparam Derived
 */
template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
class PartMgrBase {
  public:
    using GainCalc_ = typename GainMgr::GainCalc_;
    using GainMgr_ = GainMgr;
    using ConstrMgr_ = ConstrMgr;

    // using Der = Derived<Gnl, GainMgr, ConstrMgr>;

  protected:
    /// @brief Reference to the hypergraph being partitioned
    const Gnl& hyprgraph;
    /// @brief Gain manager for computing and managing gains
    GainMgr& gain_mgr;
    /// @brief Constraint manager for validating partition constraints
    ConstrMgr& validator;
    /// @brief Number of partitions
    size_t num_parts;
    // std::vector<std::uint8_t> snapshot;
    // std::vector<std::uint8_t> part;

  public:
    int total_cost{};

    /**
     * @brief Construct a new Part Mgr Base object
     *
     * @param[in] hyprgraph
     * @param[in,out] gain_mgr
     * @param[in,out] constr_mgr
     * @param[in] num_parts
     */
    PartMgrBase(const Gnl& hyprgraph, GainMgr& gain_mgr, ConstrMgr& constr_mgr, size_t num_parts)
        : hyprgraph{hyprgraph}, gain_mgr{gain_mgr}, validator{constr_mgr}, num_parts{num_parts} {}

    /**
     * @brief Initializes the partition manager with the given partition.
     *
     * @param[in,out] part The partition vector to initialize.
     */
    void init(std::span<std::uint8_t> part);

    /**
     * @brief Legalizes the partition to satisfy balance constraints.
     *
     * @param[in,out] part The partition to legalize.
     * @return LegalCheck The result of the legality check.
     */
    auto legalize(std::span<std::uint8_t> part) -> LegalCheck;

    /**
     * @brief Optimizes the partition using the FM algorithm.
     *
     * @param[in,out] part The partition to optimize.
     */
    void optimize(std::span<std::uint8_t> part);

  protected:
    /**
     * @brief Performs a single pass of the FM optimization algorithm.
     *
     * Overridable hook of the Template Method: derived managers vary the
     * pass behaviour (e.g. `NNPartMgr` stops at the first negative-gain
     * move instead of snapshotting and rolling back).
     *
     * @param[in,out] part The partition to optimize.
     */
    virtual void _optimize_1pass(std::span<std::uint8_t> part);

  private:
    /**
     * @brief Performs a final check on the partitioning based on the given partition information.
     *
     * @param[in] part The partition information to check.
     */
    auto final_check(std::span<const std::uint8_t> part) -> bool {
        return this->validator.final_check(part);
    }

    /**
     * @brief Takes a snapshot of the current partition state.
     *
     * **Memento pattern**: `take_snapshot` captures the partition state for
     * rollback; `restore_part` rolls back to it (used for backtracking when a
     * move yields negative gain).
     *
     * @param[in] part The current partition to snapshot.
     * @return std::vector<std::uint8_t> The snapshot data.
     */
    auto take_snapshot(std::span<const std::uint8_t> part) -> std::vector<std::uint8_t> {
        // const auto N = part.size();
        // auto snapshot = std::vector<std::uint8_t>(N, 0U);
        // // snapshot.reserve(N);
        // for (auto i = 0U; i != N; ++i)
        // {
        //     snapshot[i] = part[i];
        // }
        auto snapshot = std::vector<std::uint8_t>(part.begin(), part.end());
        return snapshot;
    }

    /**
     * @brief Restores the partition from a previously saved snapshot.
     *
     * **Memento pattern**: `restore_part` rolls the partition back to a state
     * previously captured by `take_snapshot` (used for backtracking when a move
     * yields negative gain).
     *
     * @param[in] snapshot The snapshot data to restore from.
     * @param[in,out] part The partition to restore.
     */
    auto restore_part(const std::vector<std::uint8_t>& snapshot, std::span<std::uint8_t> part)
        -> void {
        // std::copy(snapshot.begin(), snapshot.end(), part.begin());
        const auto N = part.size();
        for (auto idx = 0U; idx != N; ++idx) {
            part[idx] = snapshot[idx];
        }
    }
};
