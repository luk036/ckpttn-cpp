#pragma once

#include <cstdint>                     // for uint8_t
#include <gsl/span>                    // for span
#include <py2cpp/dict.hpp>             // for dict
#include <py2cpp/set.hpp>              // for set
#include <type_traits>                 // for move
#include <vector>                      // for vector
#include <xnetwork/classes/graph.hpp>  // for SimpleGraph, Graph, Graph<>::n...

#include "array_like.hpp"  // for shift_array
#include "netlist.hpp"     // for Netlist, Netlist<>::nodeview_t

// using node_t = int;

// struct PartInfo
// {
//     std::vector<std::uint8_t> part;
//     py::set<node_t> extern_nets;
// };

/**
 * @brief HierNetlist
 *
 * HierNetlist is implemented by xnetwork::Graph, which is a networkx-like graph.
 *
 * @tparam graph_t
 */
template <typename graph_t> class HierNetlist : public Netlist<graph_t> {
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

    /**
     * @brief Construct a new Hier Netlist object
     *
     * @param[in] gr
     * @param[in] modules
     * @param[in] nets
     */
    HierNetlist(graph_t gr, const nodeview_t& modules, const nodeview_t& nets);

    /**
     * @brief projection down
     *
     * @param[in] part
     * @param[out] part_down
     */
    void projection_down(gsl::span<const std::uint8_t> part,
                         gsl::span<std::uint8_t> part_down) const;

    /**
     * @brief projection up
     *
     * @param[in] part
     * @param[out] part_up
     */
    void projection_up(gsl::span<const std::uint8_t> part, gsl::span<std::uint8_t> part_up) const;

    /**
     * @brief Get the net weight
     *
     * @return int
     */
    auto get_net_weight(const node_t& net) const -> int {
        return this->net_weight.empty() ? 1 : this->net_weight[net];
    }
};

template <typename graph_t>
HierNetlist<graph_t>::HierNetlist(graph_t gr, const nodeview_t& modules, const nodeview_t& nets)
    : Netlist<graph_t>{std::move(gr), modules, nets} {}

// template <typename graph_t>
// HierNetlist<graph_t>::HierNetlist(graph_t gr, uint32_t numModules, uint32_t numNets)
//     : Netlist<graph_t> {std::move(gr), py::range<uint32_t>(numModules),
//           py::range<uint32_t>(numModules, numModules + numNets)}
// {
// }

using SimpleHierNetlist = HierNetlist<xnetwork::SimpleGraph>;
