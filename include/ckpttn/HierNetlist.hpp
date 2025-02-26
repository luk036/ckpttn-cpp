#pragma once

#include <cstdint>                     // for uint8_t
#include <py2cpp/dict.hpp>             // for dict
#include <py2cpp/set.hpp>              // for set
#include <span>                        // for span
#include <type_traits>                 // for move
#include <vector>                      // for vector
#include <xnetwork/classes/graph.hpp>  // for SimpleGraph, Graph, Graph<>::n...

#include "array_like.hpp"  // for ShiftArray
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
 * HierNetlist is implemented by xnetwork::Graph, which is a networkx-like
 * graph.
 *
 * @tparam graph_t
 */
template <typename graph_t> class HierNetlist : public Netlist<graph_t> {
  public:
    using nodeview_t = typename graph_t::nodeview_t;
    using node_t = typename graph_t::node_t;
    using index_t = typename nodeview_t::key_type;

    /* For multi-level algorithms */
    const Netlist<graph_t> *parent;
    std::vector<node_t> node_up_map;
    std::vector<node_t> node_down_map;
    py::dict<index_t, node_t> cluster_down_map;
    ShiftArray<std::vector<uint32_t>> net_weight{};

    /**
     * @brief Constructs a new HierNetlist object.
     *
     * @param[in] gr The graph object to be used for the HierNetlist.
     * @param[in] modules The nodeview of modules for the HierNetlist.
     * @param[in] nets The nodeview of nets for the HierNetlist.
     */
    HierNetlist(graph_t gr, const nodeview_t &modules, const nodeview_t &nets);

    /**
     * @brief Projection down
     *
     * Projects a part down to a lower level of the hierarchy.
     *
     * @param[in] part The part to be projected down.
     * @param[out] part_down The projected part at the lower level.
     */
    void projection_down(std::span<const std::uint8_t> part,
                         std::span<std::uint8_t> part_down) const;

    /**
     * @brief Projects a part up to a higher level of the hierarchy.
     *
     * @param[in] part The part to be projected up.
     * @param[out] part_up The projected part at the higher level.
     */
    void projection_up(std::span<const std::uint8_t> part, std::span<std::uint8_t> part_up) const;

    /**
     * @brief Returns the weight of the specified net.
     *
     * If the net_weight array is empty, the default net weight of 1 is returned.
     * Otherwise, the weight of the specified net is returned from the net_weight array.
     *
     * @param net The net for which to retrieve the weight.
     * @return The weight of the specified net.
     */
    auto get_net_weight(const node_t &net) const -> uint32_t {
        return this->net_weight.empty() ? 1U : this->net_weight[net];
    }
};

/**
 * @brief Constructs a new HierNetlist object.
 *
 * @param[in] gr The graph object to be used for the HierNetlist.
 * @param[in] modules The nodeview of modules for the HierNetlist.
 * @param[in] nets The nodeview of nets for the HierNetlist.
 */
template <typename graph_t>
HierNetlist<graph_t>::HierNetlist(graph_t gr, const nodeview_t &modules, const nodeview_t &nets)
    : Netlist<graph_t>{std::move(gr), modules, nets} {}

// template <typename graph_t>
// HierNetlist<graph_t>::HierNetlist(graph_t gr, uint32_t numModules, uint32_t
// numNets)
//     : Netlist<graph_t> {std::move(gr), py::range<uint32_t>(numModules),
//           py::range<uint32_t>(numModules, numModules + numNets)}
// {
// }

using SimpleHierNetlist = HierNetlist<xnetwork::SimpleGraph>;
