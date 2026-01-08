#pragma once

// #include <cstddef>   // for byte
#include <cstdint>             // for uint8_t
#include <mywheel/dllist.hpp>  // for Dllink
#include <span>                // for span
#include <utility>             // for pair
#include <vector>              // for vector

#include "FMPmrConfig.hpp"
// #include "moveinfo.hpp"  // for MoveInfo

// forward declare
template <typename Gnl> class FMBiGainMgr;
template <typename Node> struct MoveInfo;
template <typename Node> struct MoveInfoV;

/**
 * @brief FMBiGainCalc
 *
 * @tparam Gnl
 */
template <typename Gnl> class FMBiGainCalc {
    friend class FMBiGainMgr<Gnl>;

  public:
    using node_t = typename Gnl::node_t;
    using Item = Dllink<std::pair<node_t, uint32_t>>;

  private:
    const Gnl &hyprgraph;
    std::vector<Item> vertex_list;
    std::vector<int> init_gain_list;
    int total_cost{0};
    uint8_t stack_buf[8192];  // TODO
    FMPmr::monotonic_buffer_resource rsrc;

  public:
    int delta_gain_w{};
    FMPmr::vector<node_t> idx_vec;
    bool special_handle_2pin_nets{true};

    /**
     * @brief Constructs a new FMBiGainCalc object.
     *
     * @param[in] hyprgraph The hypergraph to use for the FMBiGainCalc object.
     */
    explicit FMBiGainCalc(const Gnl &hyprgraph, std::uint8_t /*num_parts*/)
        : hyprgraph{hyprgraph},
          vertex_list(hyprgraph.number_of_modules()),
          init_gain_list(hyprgraph.number_of_modules(), 0),
          rsrc(stack_buf, sizeof stack_buf),
          idx_vec(&rsrc) {
        for (const auto &v : this->hyprgraph) {
            this->vertex_list[v].data = std::make_pair(v, uint32_t(0));
        }
    }

    /**
     * @brief Initializes the FMBiGainCalc object.
     *
     * This function initializes the FMBiGainCalc object by resetting the total cost, vertex list,
     * and initial gain list. It then calls the _init_gain function for each net in the hypergraph
     * to initialize the gain values.
     *
     * @param[in] part The partition information.
     * @return The total cost of the initial partition.
     */
    auto init(std::span<const std::uint8_t> part) -> int {
        this->total_cost = 0;
        for (auto &vlink : this->vertex_list) {
            vlink.data.second = 0U;
        }
        for (auto &elem : this->init_gain_list) {
            elem = 0;
        }
        for (const auto &net : this->hyprgraph.nets) {
            this->_init_gain(net, part);
        }
        return this->total_cost;
    }

    /**
     * @brief This function does nothing in 2-way partitioning.
     */
    auto update_move_init() -> void {
        // nothing to do in 2-way partitioning
    }

    /**
     * @brief Initializes the index vector for a given vertex and net.
     *
     * This function is used to initialize the index vector for a given vertex and net in the
     * FMBiGainCalc object.
     *
     * @param[in] v The vertex to initialize the index vector for.
     * @param[in] net The net to initialize the index vector for.
     */
    void init_idx_vec(const node_t &module, const node_t &net);

    /**
     * @brief Update a 2-pin net during a move operation.
     *
     * This function updates the gain values for a 2-pin net when a vertex is moved during a
     * partitioning operation.
     *
     * @param[in] part The current partition information.
     * @param[in] move_info The information about the move being performed.
     * @return The vertex that was moved.
     */
    auto update_move_2pin_net(std::span<const std::uint8_t> part,
                              const MoveInfo<node_t> &move_info) -> node_t;

    /**
     * @brief Update the gain values for a 3-pin net during a move operation.
     *
     * This function updates the gain values for a 3-pin net when a vertex is moved during a
     * partitioning operation.
     *
     * @param[in] part The current partition information.
     * @param[in] move_info The information about the move being performed.
     * @return A vector of integers representing the updated gain values for the net.
     */
    auto update_move_3pin_net(std::span<const std::uint8_t> part,
                              const MoveInfo<node_t> &move_info) -> std::vector<int>;

    /**
     * @brief Update the gain values for a general net during a move operation.
     *
     * This function updates the gain values for a general net (with more than 3 pins) when a vertex
     * is moved during a partitioning operation.
     *
     * @param[in] part The current partition information.
     * @param[in] move_info The information about the move being performed.
     * @return A vector of integers representing the updated gain values for the net.
     */
    auto update_move_general_net(std::span<const std::uint8_t> part,
                                 const MoveInfo<node_t> &move_info) -> std::vector<int>;

  private:
    /**
     * @brief Modifies the gain value for the given vertex.
     *
     * This function is used to update the gain value for the given vertex in the FMBiGainCalc
     * object. The gain value is increased or decreased by the specified weight.
     *
     * @param[in] w The vertex to modify the gain for.
     * @param[in] weight The amount to modify the gain by (positive to increase, negative to
     * decrease).
     */
    auto _modify_gain(const node_t &w, int weight) -> void {
        // this->vertex_list[node_w].data.second += weight;
        this->init_gain_list[w] += weight;
    }

    /**
     * @brief Increases the gain value for the given vertex.
     *
     * This function is used to update the gain value for the given vertex in the FMBiGainCalc
     * object. The gain value is increased by the specified weight.
     *
     * @param[in] w The vertex to increase the gain for.
     * @param[in] weight The amount to increase the gain by.
     */
    auto _increase_gain(const node_t &w, uint32_t weight) -> void {
        // this->vertex_list[w].data.second += weight;
        this->init_gain_list[w] += weight;
    }

    /**
     * @brief Decreases the gain value for the given vertex.
     *
     * This function is used to update the gain value for the given vertex in the FMBiGainCalc
     * object. The gain value is decreased by the specified weight.
     *
     * @param[in] w The vertex to decrease the gain for.
     * @param[in] weight The amount to decrease the gain by.
     */
    auto _decrease_gain(const node_t &w, uint32_t weight) -> void {
        // this->vertex_list[w].data.second += weight;
        this->init_gain_list[w] -= weight;
    }

    /**
     * @brief Initializes the gain values for a net.
     *
     * This function initializes the gain values for a net based on the given net and partition
     * information.
     *
     * @param[in] net The net for which to initialize the gain values.
     * @param[in] part The current partition information.
     */
    auto _init_gain(const node_t &net, std::span<const std::uint8_t> part) -> void;

    /**
     * @brief Initializes the gain values for a 2-pin net.
     *
     * This function initializes the gain values for a 2-pin net based on the given net and
     * partition information.
     *
     * @param[in] net The net for which to initialize the gain values.
     * @param[in] part The current partition information.
     */
    auto _init_gain_2pin_net(const node_t &net, std::span<const std::uint8_t> part) -> void;

    /**
     * @brief Initializes the gain values for a 3-pin net.
     *
     * This function initializes the gain values for a 3-pin net based on the given net and
     * partition information.
     *
     * @param[in] net The net for which to initialize the gain values.
     * @param[in] part The current partition information.
     */
    auto _init_gain_3pin_net(const node_t &net, std::span<const std::uint8_t> part) -> void;

    /**
     * @brief Initializes the gain values for a general net.
     *
     * This function initializes the gain values for a general net based on the given net and
     * partition information.
     *
     * @param[in] net The net for which to initialize the gain values.
     * @param[in] part The current partition information.
     */
    auto _init_gain_general_net(const node_t &net, std::span<const std::uint8_t> part) -> void;
};
