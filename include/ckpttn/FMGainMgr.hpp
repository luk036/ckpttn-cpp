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
    const Gnl &hgr;
    std::vector<BPQueue<node_t>> gain_bucket;
    std::uint8_t num_parts;

  public:
    GainCalc gain_calc;

    // int total_cost;

    // FMGainMgr(FMGainMgr&&) = default;

    /**
     * @brief Construct a new FMGainMgr object
     *
     * @param[in] hgr
     * @param[in] num_parts
     */
    FMGainMgr(const Gnl &hgr, std::uint8_t num_parts);

    /**
     * @brief
     *
     * @param[in] part
     */
    auto init(gsl::span<const std::uint8_t> part) -> int;

    /**
     * @brief
     *
     * @param[in] to_part
     * @return true
     * @return false
     */
    auto is_empty_togo(uint8_t to_part) const -> bool {
        return this->gain_bucket[to_part].is_empty();
    }

    // /**
    //  * @brief
    //  *
    //  * @return true
    //  * @return false
    //  */
    // auto is_empty() const -> bool {
    //   return std::all_of(this->gain_bucket.cbegin(),
    //   this->gain_bucket.cend(),
    //                      [](const auto &bckt) { return bckt.is_empty(); });
    // }
    auto is_empty() const -> bool;

    /**
     * @brief
     *
     * @param[in] part
     * @return std::pair<MoveInfoV<node_t>, int>
     */
    auto select(gsl::span<const std::uint8_t> part) -> std::pair<MoveInfoV<node_t>, int>;

    /**
     * @brief
     *
     * @param[in] to_part
     * @return std::pair<node_t, int>
     */
    auto select_togo(uint8_t to_part) -> std::pair<node_t, int>;

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info_v
     */
    auto update_move(gsl::span<const std::uint8_t> part, const MoveInfoV<node_t> &move_info_v)
        -> void;

  private:
    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     */
    auto _update_move_2pin_net(gsl::span<const std::uint8_t> part,
                               const MoveInfo<node_t> &move_info) -> void;

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     */
    auto _update_move_3pin_net(gsl::span<const std::uint8_t> part,
                               const MoveInfo<node_t> &move_info) -> void;

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     */
    auto _update_move_general_net(gsl::span<const std::uint8_t> part,
                                  const MoveInfo<node_t> &move_info) -> void;
};
