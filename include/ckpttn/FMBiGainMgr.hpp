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

    /**
     * @brief Construct a new FMBiGainMgr object
     *
     * @param[in] hyprgraph The hypergraph to be used for the FMBiGainMgr object
     */
    explicit FMBiGainMgr(const Gnl &hyprgraph) : Base{hyprgraph, 2} {}

    /**
     * @brief Constructs a new FMBiGainMgr object with the given hypergraph.
     *
     * @param[in] hyprgraph The hypergraph to be used for the FMBiGainMgr object.
     */
    FMBiGainMgr(const Gnl &hyprgraph, std::uint8_t /* num_parts */) : Base{hyprgraph, 2} {}

    /**
     * @brief Initializes the FMBiGainMgr object with the given partition.
     *
     * @param[in] part The partition to initialize the FMBiGainMgr object with.
     * @return int The result of the initialization.
     */
    auto init(gsl::span<const std::uint8_t> part) -> int;

    /**
     * @brief Modifies the key for the given vertex in the gain bucket of the opposite partition.
     *
     * @param[in] w The vertex whose key is to be modified.
     * @param[in] part_w The partition that the vertex belongs to.
     * @param[in] key The new key value to be set for the vertex.
     */
    auto modify_key(const node_t &w, std::uint8_t part_w, int key) -> void {
        this->gain_bucket[1 - part_w].modify_key(this->gain_calc.vertex_list[w], key);
    }

    /**
     * @brief Updates the move information for the given vertex and gain.
     *
     * @param[in] move_info_v The move information for the vertex.
     * @param[in] gain The gain associated with the move.
     */
    auto update_move_v(const MoveInfoV<node_t> &move_info_v, int gain) -> void {
        // this->vertex_list[v].data.second -= 2 * gain;
        // auto [from_part, _ = move_info_v;
        this->_set_key(move_info_v.from_part, move_info_v.v, -gain);
    }

    /**
     * @brief Locks the vertex in the specified partition.
     *
     * @param[in] whichPart The partition to lock the vertex in.
     * @param[in] v The vertex to lock.
     */
    auto lock(uint8_t whichPart, const node_t &v) -> void {
        auto &vlink = this->gain_calc.vertex_list[v];
        this->gain_bucket[whichPart].detach(vlink);
        vlink.lock();
    }

    /**
     * @brief Locks the vertex in the opposite partition from the specified partition.
     *
     * @param[in] from_part The partition that the vertex is currently in.
     * @param[in] v The vertex to lock.
     */
    auto lock_all(uint8_t from_part, const node_t &v) -> void { this->lock(1 - from_part, v); }

  private:
    /**
     * @brief Sets the key for the given vertex in the specified partition's gain bucket.
     *
     * @param[in] whichPart The partition to set the key in.
     * @param[in] v The vertex whose key is to be set.
     * @param[in] key The new key value to be set for the vertex.
     */
    auto _set_key(uint8_t whichPart, const node_t &v, int key) -> void {
        this->gain_bucket[whichPart].set_key(this->gain_calc.vertex_list[v], key);
    }
};
