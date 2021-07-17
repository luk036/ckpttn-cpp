#pragma once

#include "FMBiGainCalc.hpp"
#include "FMGainMgr.hpp"
#include <gsl/span>

// struct FMBiGainMgr;

/*!
 * @brief FMBiGainMgr
 *
 */
class FMBiGainMgr : public FMGainMgr<FMBiGainCalc, FMBiGainMgr>
{
  public:
    using Base = FMGainMgr<FMBiGainCalc, FMBiGainMgr>;
    using GainCalc_ = FMBiGainCalc;
    using node_t = typename SimpleNetlist::node_t;

    explicit FMBiGainMgr(const SimpleNetlist& H)
        : Base {H, 2}
    {
    }

    /*!
     * @brief Construct a new FMBiGainMgr object
     *
     * @param[in] H
     */
    FMBiGainMgr(const SimpleNetlist& H, std::uint8_t /* K */)
        : Base {H, 2}
    {
    }

    /*!
     * @brief
     *
     * @param[in] part
     * @return int
     */
    auto init(gsl::span<const std::uint8_t> part) -> int;

    /*!
     * @brief (needed by base class)
     *
     * @param[in] w
     * @param[in] part_w
     * @param[in] key
     */
    auto modify_key(const node_t& w, std::uint8_t part_w, int key) -> void
    {
        this->gainbucket[1 - part_w].modify_key(
            this->gainCalc.vertex_list[w], key);
    }

    /*!
     * @brief
     *
     * @param[in] move_info_v
     * @param[in] gain
     */
    auto update_move_v(const MoveInfoV<node_t>& move_info_v, int gain) -> void
    {
        // this->vertex_list[v].data.second -= 2 * gain;
        // auto [fromPart, _ = move_info_v;
        this->_set_key(move_info_v.fromPart, move_info_v.v, -gain);
    }

    /*!
     * @brief lock
     *
     * @param[in] whichPart
     * @param[in] v
     */
    auto lock(uint8_t whichPart, const node_t& v) -> void
    {
        auto& vlink = this->gainCalc.vertex_list[v];
        this->gainbucket[whichPart].detach(vlink);
        vlink.lock();
    }

    /*!
     * @brief lock_all
     *
     * @param[in] fromPart
     * @param[in] v
     */
    auto lock_all(uint8_t fromPart, const node_t& v) -> void
    {
        this->lock(1 - fromPart, v);
    }

  private:
    /*!
     * @brief Set the key object
     *
     * @param[in] whichPart
     * @param[in] v
     * @param[in] key
     */
    auto _set_key(uint8_t whichPart, const node_t& v, int key) -> void
    {
        this->gainbucket[whichPart].set_key(this->gainCalc.vertex_list[v], key);
    }
};
