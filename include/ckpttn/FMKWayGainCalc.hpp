#pragma once

// #include <algorithm>   // for fill
#include <cstdint>      // for uint8_t
#include <gsl/span>     // for span
#include <type_traits>  // for move
#include <utility>      // for pair
#include <vector>       // for vector

#include "FMPmrConfig.hpp"
#include "dllist.hpp"  // for Dllink
#include "robin.hpp"   // for fun::Robin<>...

// forward declare
template <typename Gnl> class FMKWayGainMgr;
template <typename Node> struct MoveInfo;
template <typename Node> struct MoveInfoV;

/**
 * @brief FMKWayGainCalc
 *
 * @tparam Gnl
 */
template <typename Gnl> class FMKWayGainCalc {
    friend class FMKWayGainMgr<Gnl>;
    using node_t = typename Gnl::node_t;
    using Item = Dllink<std::pair<node_t, uint32_t>>;

  private:
    const Gnl &hgr;
    std::uint8_t num_parts;
    fun::Robin<std::uint8_t> rr;
    // size_t num_modules;
    int total_cost{0};
    uint8_t stack_buf[20000];
    FMPmr::monotonic_buffer_resource rsrc;
    std::vector<std::vector<Item>> vertex_list;
    std::vector<std::vector<int>> init_gain_list;
    FMPmr::vector<int> delta_gain_v;

  public:
    FMPmr::vector<int> delta_gain_w;
    FMPmr::vector<node_t> idx_vec;
    bool special_handle_2pin_nets{true};  // @TODO should be template parameter

    /**
     * @brief Construct a new FMKWayGainCalc object
     *
     * @param[in] hgr Netlist
     * @param[in] num_parts number of partitions
     */
    FMKWayGainCalc(const Gnl &hgr, std::uint8_t num_parts)
        : hgr{hgr},
          num_parts{num_parts},
          rr{num_parts},
          rsrc(stack_buf, sizeof stack_buf),
          vertex_list{},
          init_gain_list(num_parts, std::vector<int>(hgr.number_of_modules(), 0)),
          delta_gain_v(num_parts, 0, &rsrc),
          delta_gain_w(num_parts, 0, &rsrc),
          idx_vec(&rsrc) {
        for (auto k = 0U; k != this->num_parts; ++k) {
            auto vec = std::vector<Item>{};
            vec.reserve(hgr.number_of_modules());
            for (const auto &v : this->hgr) {
                vec.emplace_back(Item(std::make_pair(v, 0)));
            }
            this->vertex_list.emplace_back(std::move(vec));
        }
    }

    // /**
    //  * @brief
    //  *
    //  * @param[in] to_part
    //  * @return Dllink*
    //  */
    // auto start_ptr(uint8_t to_part) -> Dllink<std::pair<node_t, int32_t>>*
    // {
    //     return &this->vertex_list[to_part][0];
    // }

    /**
     * @brief
     *
     * @param[in] part
     */
    auto init(gsl::span<const std::uint8_t> part) -> int {
        this->total_cost = 0;
        for (auto &vec : this->vertex_list) {
            for (auto &vlink : vec) {
                vlink.data.second = 0U;
            }
        }
        for (auto &vec : this->init_gain_list) {
            for (auto &elem : vec) {
                elem = 0;
            }
        }
        for (const auto &net : this->hgr.nets) {
            this->_init_gain(net, part);
        }
        return this->total_cost;
    }

    // /**
    //  * @brief
    //  *
    //  */
    // auto update_move_init() -> void {
    //   std::fill(this->delta_gain_v.begin(), this->delta_gain_v.end(), 0);
    // }
    auto update_move_init() -> void;

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     * @return node_t
     */
    auto update_move_2pin_net(gsl::span<const std::uint8_t> part, const MoveInfo<node_t> &move_info)
        -> node_t;

    /**
     * @brief
     *
     * @param[in] v
     * @param[in] net
     */
    void init_idx_vec(const node_t &v, const node_t &net);

    using ret_info = std::vector<std::vector<int>>;

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     * @return ret_info
     */
    auto update_move_3pin_net(gsl::span<const std::uint8_t> part, const MoveInfo<node_t> &move_info)
        -> ret_info;

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     * @return ret_info
     */
    auto update_move_general_net(gsl::span<const std::uint8_t> part,
                                 const MoveInfo<node_t> &move_info) -> ret_info;

  private:
    /**
     * @brief
     *
     * @param[in] v
     * @param[in] part_v
     * @param[in] weight
     */
    auto _modify_gain(const node_t &v, std::uint8_t part_v, int weight) -> void {
        for (const auto &k : this->rr.exclude(part_v)) {
            // this->vertex_list[k][v].data.second += weight;
            this->init_gain_list[k][v] += weight;
        }
    }

    /**
     * @brief
     *
     * @param[in] v
     * @param[in] part_v
     * @param[in] weight
     */
    auto _increase_gain(const node_t &v, std::uint8_t part_v, uint32_t weight) -> void {
        for (const auto &k : this->rr.exclude(part_v)) {
            // this->vertex_list[k][v].data.second += weight;
            this->init_gain_list[k][v] += weight;
        }
    }

    /**
     * @brief
     *
     * @param[in] v
     * @param[in] part_v
     * @param[in] weight
     */
    auto _decrease_gain(const node_t &v, std::uint8_t part_v, uint32_t weight) -> void {
        for (const auto &k : this->rr.exclude(part_v)) {
            // this->vertex_list[k][v].data.second += weight;
            this->init_gain_list[k][v] -= weight;
        }
    }

    /**
     * @brief
     *
     * @param[in] net
     * @param[in] part
     */
    auto _init_gain(const node_t &net, gsl::span<const std::uint8_t> part) -> void;

    /**
     * @brief
     *
     * @param[in] net
     * @param[in] part
     */
    auto _init_gain_2pin_net(const node_t &net, gsl::span<const std::uint8_t> part) -> void;

    /**
     * @brief
     *
     * @param[in] net
     * @param[in] part
     */
    auto _init_gain_3pin_net(const node_t &net, gsl::span<const std::uint8_t> part) -> void;

    /**
     * @brief
     *
     * @param[in] net
     * @param[in] part
     */
    auto _init_gain_general_net(const node_t &net, gsl::span<const std::uint8_t> part) -> void;
};
