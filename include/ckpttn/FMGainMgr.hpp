#pragma once

#include <algorithm>  // for all_of
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
    Derived& self = *static_cast<Derived*>(this);
    using node_t = typename Gnl::node_t;
    // friend Derived;
    using Item = Dllink<std::pair<node_t, uint32_t>>;

  protected:
    Dllist<std::pair<node_t, uint32_t>> waitinglist{std::make_pair(node_t{}, uint32_t(0))};
    const Gnl& hgr;
    std::vector<BPQueue<node_t>> gainbucket;
    // size_t pmax;
    std::uint8_t num_parts;

  public:
    GainCalc gain_calc;

    // int totalcost;

    // FMGainMgr(FMGainMgr&&) = default;

    /**
     * @brief Construct a new FMGainMgr object
     *
     * @param[in] hgr
     * @param[in] num_parts
     */
    FMGainMgr(const Gnl& hgr, std::uint8_t num_parts);

    /**
     * @brief
     *
     * @param[in] part
     */
    auto init(gsl::span<const std::uint8_t> part) -> int;

    /**
     * @brief
     *
     * @param[in] toPart
     * @return true
     * @return false
     */
    [[nodiscard]] auto is_empty_togo(uint8_t toPart) const -> bool {
        return this->gainbucket[toPart].is_empty();
    }

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    [[nodiscard]] auto is_empty() const -> bool {
        return std::all_of(this->gainbucket.cbegin(), this->gainbucket.cend(),
                           [&](const auto& bckt) { return bckt.is_empty(); });
    }

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
     * @param[in] toPart
     * @return std::pair<node_t, int>
     */
    auto select_togo(uint8_t toPart) -> std::pair<node_t, int>;

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info_v
     */
    auto update_move(gsl::span<const std::uint8_t> part, const MoveInfoV<node_t>& move_info_v)
        -> void;

  private:
    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     */
    auto _update_move_2pin_net(gsl::span<const std::uint8_t> part,
                               const MoveInfo<node_t>& move_info) -> void;

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     */
    auto _update_move_3pin_net(gsl::span<const std::uint8_t> part,
                               const MoveInfo<node_t>& move_info) -> void;

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     */
    auto _update_move_general_net(gsl::span<const std::uint8_t> part,
                                  const MoveInfo<node_t>& move_info) -> void;
};
