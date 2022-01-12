#pragma once

#include <cstddef>           // for byte
#include <cstdint>           // for uint8_t
#include <gsl/span>          // for span
#include <py2cpp/range.hpp>  // for _iterator
#include <utility>           // for pair
#include <vector>            // for vector
#include <xnetwork/classes/graph.hpp>

#include "FMPmrConfig.hpp"
#include "dllist.hpp"    // for dllink
#include "moveinfo.hpp"  // for MoveInfo
#include "netlist.hpp"   // for SimpleN...

// struct FMBiGainMgr;

/**
 * @brief FMBiGainCalc
 *
 */
class FMBiGainCalc {
    friend class FMBiGainMgr;

  public:
    using node_t = typename xnetwork::SimpleGraph::node_t;
    using Item = dllink<std::pair<node_t, uint32_t>>;

  private:
    const SimpleNetlist& H;
    std::vector<Item> vertex_list;
    int totalcost{0};
    std::byte StackBuf[8192];  // ???
    FMPmr::monotonic_buffer_resource rsrc;

  public:
    int deltaGainW{};
    FMPmr::vector<node_t> IdVec;
    bool special_handle_2pin_nets{true};

    /**
     * @brief Construct a new FMBiGainCalc object
     *
     * @param[in] H
     */
    explicit FMBiGainCalc(const SimpleNetlist& H, std::uint8_t /*K*/)
        : H{H}, vertex_list(H.number_of_modules()), rsrc(StackBuf, sizeof StackBuf), IdVec(&rsrc) {
        for (const auto& v : this->H) {
            this->vertex_list[v].data = std::pair{v, int32_t(0)};
        }
    }

    /**
     * @brief
     *
     * @param[in] part
     */
    auto init(gsl::span<const std::uint8_t> part) -> int {
        this->totalcost = 0;
        for (auto& vlink : this->vertex_list) {
            vlink.data.second = 0;
        }
        for (const auto& net : this->H.nets) {
            this->_init_gain(net, part);
        }
        return this->totalcost;
    }

    /**
     * @brief update move init
     *
     */
    auto update_move_init() -> void {
        // nothing to do in 2-way partitioning
    }

    void init_IdVec(const node_t& v, const node_t& net);

    /**
     * @brief
     *
     * @param[in] part
     * @param[in] move_info
     * @return node_t
     */
    auto update_move_2pin_net(gsl::span<const std::uint8_t> part, const MoveInfo<node_t>& move_info)
        -> node_t;

    /**
     * @brief update move 3-pin net
     *
     * @param[in] part
     * @param[in] move_info
     * @return ret_info
     */
    auto update_move_3pin_net(gsl::span<const std::uint8_t> part, const MoveInfo<node_t>& move_info)
        -> std::vector<int>;

    /**
     * @brief update move general net
     *
     * @param[in] part
     * @param[in] move_info
     * @return ret_info
     */
    auto update_move_general_net(gsl::span<const std::uint8_t> part,
                                 const MoveInfo<node_t>& move_info) -> std::vector<int>;

  private:
    /**
     * @brief
     *
     * @param[in] w
     * @param[in] weight
     */
    auto _modify_gain(const node_t& w, unsigned int weight) -> void {
        this->vertex_list[w].data.second += weight;
    }

    /**
     * @brief
     *
     * @tparam Ts
     * @param[in] weight
     * @param[in] w
     */
    template <typename... Ts> auto _modify_gain_va(unsigned int weight, Ts... w) -> void {
        ((this->vertex_list[w].data.second += weight), ...);
    }

    // /**
    //  * @brief
    //  *
    //  * @tparam Ts
    //  * @param[in] weight
    //  * @param[in] w
    //  */
    // auto _modify_gain_va(unsigned int weight, const node_t& w1) -> void
    // {
    //     this->vertex_list[w1].data.second += weight;
    // }

    // /**
    //  * @brief
    //  *
    //  * @tparam Ts
    //  * @param[in] weight
    //  * @param[in] w
    //  */
    // auto _modify_gain_va(unsigned int weight, const node_t& w1, const node_t& w2) ->
    // void
    // {
    //     this->vertex_list[w1].data.second += weight;
    //     this->vertex_list[w2].data.second += weight;
    // }

    // /**
    //  * @brief
    //  *
    //  * @tparam Ts
    //  * @param[in] weight
    //  * @param[in] w
    //  */
    // auto _modify_gain_va(unsigned int weight, const node_t& w1, const node_t& w2,
    //     const node_t& w3) -> void
    // {
    //     this->vertex_list[w1].data.second += weight;
    //     this->vertex_list[w2].data.second += weight;
    //     this->vertex_list[w3].data.second += weight;
    // }

    /**
     * @brief
     *
     * @param[in] net
     * @param[in] part
     */
    auto _init_gain(const node_t& net, gsl::span<const std::uint8_t> part) -> void;

    /**
     * @brief
     *
     * @param[in] net
     * @param[in] part
     */
    auto _init_gain_2pin_net(const node_t& net, gsl::span<const std::uint8_t> part) -> void;

    /**
     * @brief
     *
     * @param[in] net
     * @param[in] part
     */
    auto _init_gain_3pin_net(const node_t& net, gsl::span<const std::uint8_t> part) -> void;

    /**
     * @brief
     *
     * @param[in] net
     * @param[in] part
     */
    auto _init_gain_general_net(const node_t& net, gsl::span<const std::uint8_t> part) -> void;
};
