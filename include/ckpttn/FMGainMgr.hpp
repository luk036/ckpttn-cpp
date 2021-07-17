#pragma once

#include "bpqueue.hpp" // import bpqueue
#include "dllist.hpp"  // import dllink
#include "netlist.hpp" // import Netlist
#include <cinttypes>
#include <gsl/span>
#include <iterator>
#include <memory>
#include <type_traits>

/*!
 * @brief
 *
 * @tparam GainCalc
 * @tparam Derived
 */
template <typename GainCalc, class Derived>
class FMGainMgr
{
    Derived& self = *static_cast<Derived*>(this);
    using node_t = typename SimpleNetlist::node_t;
    // friend Derived;
    using Item = dllink<std::pair<node_t, uint32_t>>;

  protected:
    Item waitinglist {std::pair {node_t {}, uint32_t(0)}};
    const SimpleNetlist& H;
    std::vector<bpqueue<node_t>> gainbucket;
    // size_t pmax;
    std::uint8_t K;

  public:
    GainCalc gainCalc;

    // int totalcost;

    // FMGainMgr(FMGainMgr&&) = default;

    /*!
     * @brief Construct a new FMGainMgr object
     *
     * @param[in] H
     * @param[in] K
     */
    FMGainMgr(const SimpleNetlist& H, std::uint8_t K);

    /*!
     * @brief
     *
     * @param[in] part
     */
    auto init(gsl::span<const std::uint8_t> part) -> int;

    /*!
     * @brief
     *
     * @param[in] toPart
     * @return true
     * @return false
     */
    [[nodiscard]] auto is_empty_togo(uint8_t toPart) const -> bool
    {
        return this->gainbucket[toPart].is_empty();
    }

    /*!
     * @brief
     *
     * @return true
     * @return false
     */
    [[nodiscard]] auto is_empty() const -> bool
    {
        return std::all_of(this->gainbucket.cbegin(), this->gainbucket.cend(),
            [&](const auto& bckt) { return bckt.is_empty(); });
    }

    /*!
     * @brief
     *
     * @param[in] part
     * @return std::tuple<MoveInfoV<node_t>, int>
     */
    auto select(gsl::span<const std::uint8_t> part)
        -> std::tuple<MoveInfoV<node_t>, int>;

    /*!
     * @brief
     *
     * @param[in] toPart
     * @return std::tuple<node_t, int>
     */
    auto select_togo(uint8_t toPart) -> std::tuple<node_t, int>;

    /*!
     * @brief
     *
     * @param[in] part
     * @param[in] move_info_v
     */
    auto update_move(gsl::span<const std::uint8_t> part,
        const MoveInfoV<node_t>& move_info_v) -> void;

  private:
    /*!
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     */
    auto _update_move_2pin_net(gsl::span<const std::uint8_t> part,
        const MoveInfo<node_t>& move_info) -> void;

    /*!
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     */
    auto _update_move_3pin_net(gsl::span<const std::uint8_t> part,
        const MoveInfo<node_t>& move_info) -> void;

    /*!
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     */
    auto _update_move_general_net(gsl::span<const std::uint8_t> part,
        const MoveInfo<node_t>& move_info) -> void;
};
