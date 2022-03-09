#pragma once

#include <gsl/span>

#include "FMBiGainCalc.hpp"
#include "FMGainMgr.hpp"
#include "moveinfo.hpp"  // for MoveInfo

// struct FMBiGainMgr;

/**
 * @brief FMBiGainMgr
 *
 * @tparam Gnl
 */
template <typename Gnl> class FMBiGainMgr
    : public FMGainMgr<Gnl, FMBiGainCalc<Gnl>, FMBiGainMgr<Gnl>> {
  public:
    using Base = FMGainMgr<Gnl, FMBiGainCalc<Gnl>, FMBiGainMgr<Gnl>>;
    using GainCalc_ = FMBiGainCalc<Gnl>;
    using node_t = typename Gnl::node_t;

    explicit FMBiGainMgr(const Gnl& hgr) : Base{hgr, 2} {}

    /**
     * @brief Construct a new FMBiGainMgr object
     *
     * @param[in] hgr
     */
    FMBiGainMgr(const Gnl& hgr, std::uint8_t /* num_parts */) : Base{hgr, 2} {}

    /**
     * @brief
     *
     * @param[in] part
     * @return int
     */
    auto init(gsl::span<const std::uint8_t> part) -> int;

    /**
     * @brief (needed by base class)
     *
     * @param[in] w
     * @param[in] part_w
     * @param[in] key
     */
    auto modify_key(const node_t& w, std::uint8_t part_w, int key) -> void {
        this->gainbucket[1 - part_w].modify_key(this->gain_calc.vertex_list[w], key);
    }

    /**
     * @brief
     *
     * @param[in] move_info_v
     * @param[in] gain
     */
    auto update_move_v(const MoveInfoV<node_t>& move_info_v, int gain) -> void {
        // this->vertex_list[v].data.second -= 2 * gain;
        // auto [from_part, _ = move_info_v;
        this->_set_key(move_info_v.from_part, move_info_v.v, -gain);
    }

    /**
     * @brief lock
     *
     * @param[in] whichPart
     * @param[in] v
     */
    auto lock(uint8_t whichPart, const node_t& v) -> void {
        auto& vlink = this->gain_calc.vertex_list[v];
        this->gainbucket[whichPart].detach(vlink);
        vlink.lock();
    }

    /**
     * @brief lock_all
     *
     * @param[in] from_part
     * @param[in] v
     */
    auto lock_all(uint8_t from_part, const node_t& v) -> void { this->lock(1 - from_part, v); }

  private:
    /**
     * @brief Set the key object
     *
     * @param[in] whichPart
     * @param[in] v
     * @param[in] key
     */
    auto _set_key(uint8_t whichPart, const node_t& v, int key) -> void {
        this->gainbucket[whichPart].set_key(this->gain_calc.vertex_list[v], key);
    }
};
