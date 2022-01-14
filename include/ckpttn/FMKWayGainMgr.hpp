#pragma once

#include <gsl/span>

#include "FMGainMgr.hpp"
#include "FMKWayGainCalc.hpp"

// forward declare
template <typename Node> struct MoveInfo;
template <typename Node> struct MoveInfoV;

/**
 * @brief FMKWayGainMgr
 *
 */
template <typename Gnl> class FMKWayGainMgr
    : public FMGainMgr<Gnl, FMKWayGainCalc<Gnl>, FMKWayGainMgr<Gnl>> {
  private:
    robin<std::uint8_t> RR;

  public:
    using Base = FMGainMgr<Gnl, FMKWayGainCalc<Gnl>, FMKWayGainMgr<Gnl>>;
    using GainCalc_ = FMKWayGainCalc<Gnl>;
    using node_t = typename Gnl::node_t;

    /**
     * @brief Construct a new FMKWayGainMgr object
     *
     * @param[in] H
     * @param[in] K
     */
    FMKWayGainMgr(const Gnl& H, std::uint8_t K) : Base{H, K}, RR{K} {}

    /**
     * @brief
     *
     * @param[in] part
     */
    auto init(gsl::span<const std::uint8_t> part) -> int;

    /**
     * @brief (needed by base class)
     *
     * @param[in] w
     * @param[in] part_w
     * @param[in] keys
     */
    auto modify_key(const node_t& w, std::uint8_t part_w, gsl::span<const int> keys) -> void {
        for (auto k : this->RR.exclude(part_w)) {
            this->gainbucket[k].modify_key(this->gainCalc.vertex_list[k][w], keys[k]);
        }
    }

    /**
     * @brief
     *
     * @param[in] move_info_v
     * @param[in] gain
     */
    auto update_move_v(const MoveInfoV<node_t>& move_info_v, int gain) -> void;

    /**
     * @brief lock
     *
     * @param[in] whichPart
     * @param[in] v
     */
    auto lock(uint8_t whichPart, const node_t& v) -> void {
        auto& vlink = this->gainCalc.vertex_list[whichPart][v];
        this->gainbucket[whichPart].detach(vlink);
        vlink.lock();
    }

    /**
     * @brief lock_all
     *
     * @param[in] v
     */
    auto lock_all(uint8_t /*fromPart*/, const node_t& v) -> void {
        // for (const auto& [vlist, bckt] :
        //     views::zip(this->gainCalc.vertex_list, this->gainbucket))
        auto bckt_it = this->gainbucket.begin();
        for (auto& vlist : this->gainCalc.vertex_list) {
            auto& vlink = vlist[v];
            bckt_it->detach(vlink);
            vlink.lock();  // lock
            ++bckt_it;
        }
    }

  private:
    /**
     * @brief Set the key object
     *
     * @param[in] whichPart
     * @param[in] v
     * @param[in] key
     */
    auto _set_key(uint8_t whichPart, const node_t& v, int key) -> void {
        this->gainbucket[whichPart].set_key(this->gainCalc.vertex_list[whichPart][v], key);
    }
};
