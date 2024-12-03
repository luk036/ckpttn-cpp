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
 * @tparam Gnl Generalized Netlist
 */
template <typename Gnl> class FMKWayGainMgr
    : public FMGainMgr<Gnl, FMKWayGainCalc<Gnl>, FMKWayGainMgr<Gnl>> {
  private:
    fun::Robin<std::uint8_t> rr;

  public:
    using Base = FMGainMgr<Gnl, FMKWayGainCalc<Gnl>, FMKWayGainMgr<Gnl>>;
    using GainCalc_ = FMKWayGainCalc<Gnl>;
    using node_t = typename Gnl::node_t;

    /**
     * @brief Constructs a new FMKWayGainMgr object.
     *
     * @param[in] hyprgraph The hypergraph to use.
     * @param[in] num_parts The number of partitions.
     */
    FMKWayGainMgr(const Gnl &hyprgraph, std::uint8_t num_parts)
        : Base{hyprgraph, num_parts}, rr{num_parts} {}

    /**
     * @brief Initializes the gain manager with the given partition information.
     *
     * @param[in] part The partition information to initialize the gain manager with.
     * @return int The result of the initialization.
     */
    auto init(gsl::span<const std::uint8_t> part) -> int;

    /**
     * @brief Modifies the key for the given vertex in the gain buckets for all partitions except
     * the given one.
     *
     * @param[in] w The vertex to modify the key for.
     * @param[in] part_w The partition that the vertex belongs to.
     * @param[in] keys The new keys to set for the vertex in each partition.
     */
    auto modify_key(const node_t &w, std::uint8_t part_w, gsl::span<const int> keys) -> void {
        for (auto k : this->rr.exclude(part_w)) {
            this->gain_bucket[k].modify_key(this->gain_calc.vertex_list[k][w], keys[k]);
        }
    }

    /**
     * @brief Updates the move information for a vertex.
     *
     * @param[in] move_info_v The move information for the vertex.
     * @param[in] gain The gain associated with the move.
     */
    auto update_move_v(const MoveInfoV<node_t> &move_info_v, int gain) -> void;

    /**
     * @brief Locks the vertex link for the given partition and vertex.
     *
     * @param[in] whichPart The partition to lock the vertex link for.
     * @param[in] v The vertex to lock the link for.
     */
    auto lock(uint8_t whichPart, const node_t &v) -> void {
        auto &vlink = this->gain_calc.vertex_list[whichPart][v];
        this->gain_bucket[whichPart].detach(vlink);
        vlink.lock();
    }

    /**
     * @brief Locks the vertex link for the given vertex in all partitions.
     *
     * @param[in] from_part The partition to exclude from locking.
     * @param[in] v The vertex to lock the link for.
     */
    auto lock_all(uint8_t /*from_part*/, const node_t &v) -> void {
        // for (const auto& [vlist, bckt] :
        //     views::zip(this->gain_calc.vertex_list, this->gain_bucket))
        auto bckt_it = this->gain_bucket.begin();
        for (auto &vlist : this->gain_calc.vertex_list) {
            auto &vlink = vlist[v];
            bckt_it->detach(vlink);
            vlink.lock();  // lock
            ++bckt_it;
        }
    }

  private:
    /**
     * @brief Sets the key for a vertex in the specified partition.
     *
     * @param[in] whichPart The partition to set the key for.
     * @param[in] v The vertex to set the key for.
     * @param[in] key The new key to set for the vertex.
     */
    auto _set_key(uint8_t whichPart, const node_t &v, int key) -> void {
        this->gain_bucket[whichPart].set_key(this->gain_calc.vertex_list[whichPart][v], key);
    }
};
