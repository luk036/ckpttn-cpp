#pragma once

// import networkx as nx
// #include <algorithm>
// #include <gsl/span>
// #include <iterator>
// #include <py2cpp/py2cpp.hpp>
// #include <utility>
// #include <vector>
// #include <xnetwork/classes/graph.hpp>
#include "netlist.hpp"
#include "array_like.hpp"

// using node_t = int;

// struct PartInfo
// {
//     std::vector<std::uint8_t> part;
//     py::set<node_t> extern_nets;
// };

/*!
 * @brief HierNetlist
 *
 * HierNetlist is implemented by xn::Graph, which is a networkx-like graph.
 *
 */
template <typename graph_t>
class HierNetlist : public Netlist<graph_t>
{
  public:
    using nodeview_t = typename graph_t::nodeview_t;
    using node_t = typename graph_t::node_t;
    using index_t = typename nodeview_t::key_type;

    /* For multi-level algorithms */
    const Netlist<graph_t>* parent;
    std::vector<node_t> node_up_map;
    std::vector<node_t> node_down_map;
    py::dict<index_t, node_t> cluster_down_map;
    shift_array<std::vector<int>> net_weight{};


    /*!
     * @brief Construct a new HierNetlist object
     *
     * @param[in] G
     * @param[in] module_list
     * @param[in] net_list
     * @param[in] module_fixed
     */
    HierNetlist(graph_t G, const nodeview_t& modules, const nodeview_t& nets);

    /**
     * @brief Construct a new Netlist object
     *
     * @param[in] G
     * @param[in] num_modules
     * @param[in] num_nets
     */
    // HierNetlist(graph_t G, uint32_t numModules, uint32_t numNets);

    /*!
     * @brief projection down
     *
     * @param[in] part
     * @param[out] part_down
     */
    void projection_down(gsl::span<const std::uint8_t> part,
        gsl::span<std::uint8_t> part_down) const;

    /*!
     * @brief projection up
     *
     * @param[in] part
     * @param[out] part_up
     */
    void projection_up(gsl::span<const std::uint8_t> part,
        gsl::span<std::uint8_t> part_up) const;

    /**
     * @brief Get the net weight
     *
     * @return int
     */
    auto get_net_weight(const node_t& net) const -> int
    {
        return this->net_weight.empty() ? 1
                                        :
                                        this->net_weight[net];
    }
};

/**
 * @brief Construct a new Netlist object
 *
 * @tparam nodeview_t
 * @tparam nodemap_t
 * @param[in] G
 * @param[in] modules
 * @param[in] nets
 */
template <typename graph_t>
HierNetlist<graph_t>::HierNetlist(
    graph_t G, const nodeview_t& modules, const nodeview_t& nets)
    : Netlist<graph_t> {std::move(G), modules, nets}
{
}

// template <typename graph_t>
// HierNetlist<graph_t>::HierNetlist(graph_t G, uint32_t numModules, uint32_t numNets)
//     : Netlist<graph_t> {std::move(G), py::range<uint32_t>(numModules),
//           py::range<uint32_t>(numModules, numModules + numNets)}
// {
// }

using SimpleHierNetlist = HierNetlist<xn::SimpleGraph>;
