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
    const Gnl &hyprgraph;
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
     * @brief Constructs a new FMKWayGainCalc object.
     *
     * @param[in] hyprgraph The netlist.
     * @param[in] num_parts The number of partitions.
     */
    FMKWayGainCalc(const Gnl &hyprgraph, std::uint8_t num_parts)
        : hyprgraph{hyprgraph},
          num_parts{num_parts},
          rr{num_parts},
          rsrc(stack_buf, sizeof stack_buf),
          vertex_list{},
          init_gain_list(num_parts, std::vector<int>(hyprgraph.number_of_modules(), 0)),
          delta_gain_v(num_parts, 0, &rsrc),
          delta_gain_w(num_parts, 0, &rsrc),
          idx_vec(&rsrc) {
        for (auto k = 0U; k != this->num_parts; ++k) {
            auto vec = std::vector<Item>{};
            vec.reserve(hyprgraph.number_of_modules());
            for (const auto &v : this->hyprgraph) {
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
     * @brief Initializes the FMKWayGainCalc object.
     *
     * This function resets the total cost, initializes the vertex list and init gain list to 0,
     * and then calls the _init_gain function for each net in the hypergraph.
     *
     * @param[in] part The partition to initialize.
     * @return The total cost after initialization.
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
        for (const auto &net : this->hyprgraph.nets) {
            this->_init_gain(net, part);
        }
        return this->total_cost;
    }

    /**
     * @brief Resets the delta gain vector to 0.
     *
     * This function is used to initialize the delta gain vector before updating the gains for a
     * move.
     */
    auto update_move_init() -> void;

    /**
     * @brief Updates the gain for a 2-pin net after a move.
     *
     * This function updates the gain for a 2-pin net after a move has been performed. It takes the
     * current partition and the move information as input, and returns the updated gain for the
     * net.
     *
     * @param[in] part The current partition.
     * @param[in] move_info The information about the move that was performed.
     * @return The updated gain for the 2-pin net.
     */
    auto update_move_2pin_net(gsl::span<const std::uint8_t> part,
                              const MoveInfo<node_t> &move_info) -> node_t;

    /**
     * @brief Initializes the index vector for a given vertex and net.
     *
     * This function is used to initialize the index vector for a vertex within a net.
     * It is likely an implementation detail of the FMKWayGainCalc class.
     *
     * @param[in] v The vertex to initialize the index vector for.
     * @param[in] net The net that the vertex belongs to.
     */
    void init_idx_vec(const node_t &v, const node_t &net);

    using ret_info = std::vector<std::vector<int>>;

    /**
     * @brief Updates the gain for a 3-pin net after a move.
     *
     * This function updates the gain for a 3-pin net after a move has been performed. It takes
     * the current partition and the move information as input, and returns the updated gain for the
     * net.
     *
     * @param[in] part The current partition.
     * @param[in] move_info The information about the move that was performed.
     * @return The updated gain for the 3-pin net.
     */
    auto update_move_3pin_net(gsl::span<const std::uint8_t> part,
                              const MoveInfo<node_t> &move_info) -> ret_info;

    /**
     * @brief Updates the gain for a general net after a move.
     *
     * This function updates the gain for a general net (with any number of pins) after a move has
     * been performed. It takes the current partition and the move information as input, and returns
     * the updated gain for the net.
     *
     * @param[in] part The current partition.
     * @param[in] move_info The information about the move that was performed.
     * @return The updated gain for the general net.
     */
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
     * @brief Modifies the gain value for a vertex in the gain list.
     *
     * This function updates the gain value for a vertex in the `init_gain_list` based on the
     * provided `part_v` and `weight` parameters. It iterates over the partitions that exclude the
     * given `part_v` and updates the gain value for the vertex in the corresponding partition.
     *
     * @param[in] v The vertex for which the gain is to be modified.
     * @param[in] part_v The partition from which the vertex is being moved.
     * @param[in] weight The weight to be added or subtracted from the gain value.
     */
    auto _modify_gain(const node_t &v, std::uint8_t part_v, int weight) -> void {
        for (const auto &k : this->rr.exclude(part_v)) {
            // this->vertex_list[k][v].data.second += weight;
            this->init_gain_list[k][v] += weight;
        }
    }

    /**
     * @brief Increases the gain value for a vertex in the gain list.
     *
     * This function updates the gain value for a vertex in the `init_gain_list` based on the
     * provided `part_v` and `weight` parameters. It iterates over the partitions that exclude the
     * given `part_v` and increases the gain value for the vertex in the corresponding partition.
     *
     * @param[in] v The vertex for which the gain is to be increased.
     * @param[in] part_v The partition from which the vertex is being moved.
     * @param[in] weight The weight to be added to the gain value.
     */
    auto _increase_gain(const node_t &v, std::uint8_t part_v, uint32_t weight) -> void {
        for (const auto &k : this->rr.exclude(part_v)) {
            // this->vertex_list[k][v].data.second += weight;
            this->init_gain_list[k][v] += weight;
        }
    }

    /**
     * @brief Decreases the gain value for a vertex in the gain list.
     *
     * This function updates the gain value for a vertex in the `init_gain_list` based on the
     * provided `part_v` and `weight` parameters. It iterates over the partitions that exclude the
     * given `part_v` and decreases the gain value for the vertex in the corresponding partition.
     *
     * @param[in] v The vertex for which the gain is to be decreased.
     * @param[in] part_v The partition from which the vertex is being moved.
     * @param[in] weight The weight to be subtracted from the gain value.
     */
    auto _decrease_gain(const node_t &v, std::uint8_t part_v, uint32_t weight) -> void {
        for (const auto &k : this->rr.exclude(part_v)) {
            // this->vertex_list[k][v].data.second += weight;
            this->init_gain_list[k][v] -= weight;
        }
    }

    /**
     * @brief Initializes the gain values for a net in the partitioning.
     *
     * This function initializes the gain values for the vertices in the given net based on the
     * current partitioning. The gain values are stored in the `init_gain_list` data structure.
     *
     * @param[in] net The net for which the gain values are to be initialized.
     * @param[in] part The current partitioning of the vertices.
     */
    auto _init_gain(const node_t &net, gsl::span<const std::uint8_t> part) -> void;

    /**
     * @brief Initializes the gain values for a 2-pin net in the partitioning.
     *
     * This function initializes the gain values for the vertices in the given 2-pin net based on
     * the current partitioning. The gain values are stored in the `init_gain_list` data structure.
     *
     * @param[in] net The 2-pin net for which the gain values are to be initialized.
     * @param[in] part The current partitioning of the vertices.
     */
    auto _init_gain_2pin_net(const node_t &net, gsl::span<const std::uint8_t> part) -> void;

    /**
     * @brief Initializes the gain values for a 3-pin net in the partitioning.
     *
     * This function initializes the gain values for the vertices in the given 3-pin net based on
     * the current partitioning. The gain values are stored in the `init_gain_list` data structure.
     *
     * @param[in] net The 3-pin net for which the gain values are to be initialized.
     * @param[in] part The current partitioning of the vertices.
     */
    auto _init_gain_3pin_net(const node_t &net, gsl::span<const std::uint8_t> part) -> void;

    /**
     * @brief Initializes the gain values for a general net in the partitioning.
     *
     * This function initializes the gain values for the vertices in the given net based on the
     * current partitioning. The gain values are stored in the `init_gain_list` data structure.
     *
     * @param[in] net The net for which the gain values are to be initialized.
     * @param[in] part The current partitioning of the vertices.
     */
    auto _init_gain_general_net(const node_t &net, gsl::span<const std::uint8_t> part) -> void;
};
