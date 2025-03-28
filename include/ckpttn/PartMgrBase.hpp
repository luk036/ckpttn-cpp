#pragma once

// **Special code for two-pin nets**
// Take a snapshot when a move make **negative** gain.
// Snapshot in the form of "interface"???

#include <cstdint>  // for uint8_t
#include <span>
#include <span>    // for span
#include <vector>  // for vector
// #include <xnetwork/classes/graph.hpp>

// forward declare
// template <typename graph_t> struct Netlist;
// using SimpleNetlist = Netlist<xnetwork::SimpleGraph>;

enum class LegalCheck;

/**
 * @brief Fiduccia-Mattheyses Partitioning Algorithm Manager Base
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
    // Der& self = *static_cast<Der*>(this);

    const Gnl &hyprgraph;
    GainMgr &gain_mgr;
    ConstrMgr &validator;
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
    PartMgrBase(const Gnl &hyprgraph, GainMgr &gain_mgr, ConstrMgr &constr_mgr, size_t num_parts)
        : hyprgraph{hyprgraph}, gain_mgr{gain_mgr}, validator{constr_mgr}, num_parts{num_parts} {}

    /**
     * @brief
     *
     * @param[in,out] part
     */
    void init(std::span<std::uint8_t> part);

    /**
     * @brief
     *
     * @param[in,out] part
     * @return LegalCheck
     */
    auto legalize(std::span<std::uint8_t> part) -> LegalCheck;

    /**
     * @brief
     *
     * @param[in,out] part
     */
    void optimize(std::span<std::uint8_t> part);

  private:
    /**
     * @brief
     *
     * @param[in,out] part
     */
    void _optimize_1pass(std::span<std::uint8_t> part);

    /**
     * @brief
     *
     * @param[in] part
     * @return std::vector<std::uint8_t>
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
     * @brief
     *
     * @param[in] snapshot
     * @param[in,out] part
     */
    auto restore_part(const std::vector<std::uint8_t> &snapshot,
                      std::span<std::uint8_t> part) -> void {
        // std::copy(snapshot.begin(), snapshot.end(), part.begin());
        const auto N = part.size();
        for (auto i = 0U; i != N; ++i) {
            part[i] = snapshot[i];
        }
    }
};
