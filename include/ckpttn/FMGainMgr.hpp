#pragma once

// #include <algorithm> // for all_of
#include <cinttypes>  // for uint8_t, uint32_t
#include <gsl/span>   // for span
#include <tuple>      // for tuple
#include <utility>    // for pair
#include <vector>     // for vector<>::const_iterator, vector

#include "bpqueue.hpp"  // for BPQueue
#include "dllist.hpp"   // for Dllink

template <typename Node> struct MoveInfo;
template <typename Node> struct MoveInfoV;

/**
 * @brief
 *
 * @tparam Gnl
 * @tparam GainCalc
 * @tparam Derived
 */
template <typename Gnl, typename GainCalc, class Derived> class FMGainMgr {
    Derived &self = *static_cast<Derived *>(this);
    using node_t = typename Gnl::node_t;
    using Item = Dllink<std::pair<node_t, uint32_t>>;

  protected:
    Dllist<std::pair<node_t, uint32_t>> waiting_list{std::make_pair(node_t{}, uint32_t(0))};
    const Gnl &hyprgraph;
    std::vector<BPQueue<node_t>> gain_bucket;
    std::uint8_t num_parts;

  public:
    GainCalc gain_calc;

    // int total_cost;

    // FMGainMgr(FMGainMgr&&) = default;

    /**
     * @brief Constructs a new FMGainMgr object.
     *
     * @param[in] hyprgraph The hypergraph to manage the gains for.
     * @param[in] num_parts The number of partitions in the hypergraph.
     */
    FMGainMgr(const Gnl &hyprgraph, std::uint8_t num_parts);

    /**
     * @brief Initializes the FMGainMgr with the given partition information.
     *
     * @param[in] part The partition information to initialize the FMGainMgr with.
     * @return int The result of the initialization.
     */
    auto init(gsl::span<const std::uint8_t> part) -> int;

    /**
     * @brief Checks if the gain bucket for the given partition is empty.
     *
     * @param[in] to_part The partition to check.
     * @return true If the gain bucket for the given partition is empty.
     * @return false If the gain bucket for the given partition is not empty.
     */
    auto is_empty_togo(uint8_t to_part) const -> bool {
        return this->gain_bucket[to_part].is_empty();
    }

    /**
     * @brief Checks if all the gain buckets are empty.
     *
     * @return true If all the gain buckets are empty.
     * @return false If any of the gain buckets are not empty.
     */
    auto is_empty() const -> bool;

    /**
     * @brief Selects a set of moves to perform on the given partition.
     *
     * @param[in] part The current partition information.
     * @return std::pair<MoveInfoV<node_t>, int> A pair containing the selected moves and the total gain of the moves.
     */
    auto select(gsl::span<const std::uint8_t> part) -> std::pair<MoveInfoV<node_t>, int>;

    /**
     * @brief Selects a node to move to the given partition.
     *
     * @param[in] to_part The partition to select a node to move to.
     * @return std::pair<node_t, int> A pair containing the selected node and the gain of moving that node.
     */
    auto select_togo(uint8_t to_part) -> std::pair<node_t, int>;

    /**
     * @brief Updates the gain information for the given set of moves.
     *
     * @param[in] part The current partition information.
     * @param[in] move_info_v The set of moves to update the gain information for.
     */
    auto update_move(gsl::span<const std::uint8_t> part, const MoveInfoV<node_t> &move_info_v)
        -> void;

  private:
    /**
     * @brief Updates the gain information for a 2-pin net after a move.
     *
     * @param[in] part The current partition information.
     * @param[in] move_info The information about the move to update the gain for.
     */
    auto _update_move_2pin_net(gsl::span<const std::uint8_t> part,
                               const MoveInfo<node_t> &move_info) -> void;

    /**
     * @brief Updates the gain information for a 3-pin net after a move.
     *
     * @param[in] part The current partition information.
     * @param[in] move_info The information about the move to update the gain for.
     */
    auto _update_move_3pin_net(gsl::span<const std::uint8_t> part,
                               const MoveInfo<node_t> &move_info) -> void;

    /**
     * @brief Updates the gain information for a general net (not 2-pin or 3-pin) after a move.
     *
     * @param[in] part The current partition information.
     * @param[in] move_info The information about the move to update the gain for.
     */
    auto _update_move_general_net(gsl::span<const std::uint8_t> part,
                                  const MoveInfo<node_t> &move_info) -> void;
};
