#include <ckpttn/HierNetlist.hpp>      // for SimpleHierNetlist, HierNetlist
#include <cstdint>                     // for uint32_t
#include <memory>                      // for unique_ptr, make_unique
#include <netlistx/netlist.hpp>        // for SimpleNetlist, index_t, Netlist
#include <netlistx/netlist_algo.hpp>   // for min_maximal_matching
#include <py2cpp/dict.hpp>             // for dict, dict<>::Base
#include <py2cpp/range.hpp>            // for _iterator, iterable_wrapper
#include <py2cpp/set.hpp>              // for set
#include <transrangers.hpp>            // for accumlate, transform, all
#include <type_traits>                 // for move
#include <unordered_map>               // for __hash_map_iterator, operator!=
#include <utility>                     // for get
#include <vector>                      // for vector<>::iterator, vector
#include <xnetwork/classes/graph.hpp>  // for Graph, Graph<>::nodeview_t

using node_t = typename SimpleNetlist::node_t;
// using namespace std;
// using namespace transrangers;

/**
 * The function creates a contraction subgraph object from a given netlist and a set of nodes to
 * exclude.
 *
 * @param[in] hyprgraph A reference to a `SimpleNetlist` object, which represents a hierarchical
 * netlist.
 * @param[in] dont_select A set of nodes that should not be selected for contraction.
 *
 * @return The function `create_contracted_subgraph` returns a `std::unique_ptr` to a
 * `SimpleHierNetlist` object.
 *
 * @todo simplify this function
 */
auto create_contracted_subgraph(const SimpleNetlist &hyprgraph, const py::set<node_t> &dont_select)
    -> std::unique_ptr<SimpleHierNetlist> {
    using namespace transrangers;

    auto weight_dict = py::dict<node_t, unsigned int>{};
    auto rng_nets = all(hyprgraph.nets);
    rng_nets([&](const auto &netcur) {
        weight_dict[*netcur]
            = accumulate(transform([&](const auto &v) { return hyprgraph.get_module_weight(v); },
                                   all(hyprgraph.gr[*netcur])),
                         0U);
        return true;
    });
    // for (const auto &net : hyprgraph.nets) {
    //   weight_dict[net] = accumulate(
    //       transform([&](const auto &v) { return hyprgraph.get_module_weight(v); },
    //                 all(hyprgraph.gr[net])),
    //       0U);
    // }

    auto S = py::set<node_t>{};
    auto dep = dont_select.copy();
    min_maximal_matching(hyprgraph, weight_dict, S, dep);

    auto module_up_map = py::dict<node_t, node_t>{};
    module_up_map.reserve(hyprgraph.number_of_modules());
    for (const auto &v : hyprgraph) {
        module_up_map[v] = v;
    }

    // auto cluster_map = py::dict<node_t, node_t> {};
    // cluster_map.reserve(S.size());
    auto node_up_dict = py::dict<node_t, index_t>{};
    auto net_up_map = py::dict<node_t, index_t>{};

    auto modules = std::vector<node_t>{};
    auto nets = std::vector<node_t>{};
    nets.reserve(hyprgraph.nets.size() - S.size());

    {  // localize C and clusters
        auto C = py::set<node_t>{};
        auto clusters = std::vector<node_t>{};
        C.reserve(3 * S.size());  // TODO
        clusters.reserve(S.size());

        for (const auto &net : hyprgraph.nets) {
            if (S.contains(net)) {
                // auto net_cur = hyprgraph.gr[net].begin();
                // auto master = *net_cur;
                clusters.push_back(net);
                for (const auto &v : hyprgraph.gr[net]) {
                    module_up_map[v] = net;
                    C.insert(v);
                }
                // cluster_map[master] = net;
            } else {
                nets.push_back(net);
            }
        }
        modules.reserve(hyprgraph.modules.size() - C.size() + clusters.size());
        for (const auto &v : hyprgraph) {
            if (C.contains(v)) {
                continue;
            }
            modules.push_back(v);
        }
        modules.insert(modules.end(), clusters.begin(), clusters.end());
    }
    // auto nodes = vector<node_t>{};
    // nodes.reserve(modules.size() + nets.size());

    // nodes.insert(nodes.end(), modules.begin(), modules.end());
    // nodes.insert(nodes.end(), nets.begin(), nets.end());
    auto numModules = uint32_t(modules.size());
    auto numNets = uint32_t(nets.size());

    {  // localize module_map and net_map
        auto module_map = py::dict<node_t, index_t>{};
        module_map.reserve(numModules);
        auto i_v = index_t(0);
        for (const auto &v : modules) {
            module_map[v] = index_t(i_v);
            ++i_v;
        }

        // auto net_map = py::dict<node_t, index_t> {};
        net_up_map.reserve(numNets);
        auto i_net = index_t(0);
        for (const auto &net : nets) {
            net_up_map[net] = index_t(i_net) + numModules;
            ++i_net;
        }

        node_up_dict.reserve(hyprgraph.number_of_modules());

        for (const auto &v : hyprgraph) {
            node_up_dict[v] = module_map[module_up_map[v]];
        }
        // for (const auto& net : nets)
        // {
        //     node_up_dict[net] = net_map[net] + numModules;
        // }
    }

    auto num_vertices = numModules + numNets;
    // auto R = py::range<node_t>(0, num_vertices);
    auto g = graph_t(num_vertices);
    // gr.add_nodes_from(nodes);
    for (const auto &v : hyprgraph) {
        for (const auto &net : hyprgraph.gr[v]) {
            if (S.contains(net)) {
                continue;
            }
            g.add_edge(node_up_dict[v], net_up_map[net]);
        }
    }
    // auto gr = py::GraphAdaptor<graph_t>(move(g));
    auto gr = std::move(g);

    auto hgr2 = std::make_unique<SimpleHierNetlist>(std::move(gr), py::range(numModules),
                                                    py::range(numModules, numModules + numNets));

    auto node_down_map = std::vector<node_t>{};
    node_down_map.resize(numModules);
    // for (const auto& [v1, v2] : node_up_dict.items())
    for (const auto &keyvalue : node_up_dict.items()) {
        auto &&v1 = std::get<0>(keyvalue);
        auto &&v2 = std::get<1>(keyvalue);
        node_down_map[v2] = v1;
    }
    auto cluster_down_map = py::dict<index_t, node_t>{};
    // cluster_down_map.reserve(cluster_map.size()); // ???
    // // for (const auto& [v, net] : cluster_map.items())
    // for (const auto& keyvalue : cluster_map.items())
    // {
    //     auto&& v = get<0>(keyvalue);
    //     auto&& net = get<1>(keyvalue);
    //     cluster_down_map[node_up_dict[v]] = net;
    // }
    for (auto &&net : S) {
        for (auto &&v : hyprgraph.gr[net]) {
            cluster_down_map[node_up_dict[v]] = net;
        }
    }

    auto module_weight = std::vector<unsigned int>{};
    module_weight.reserve(numModules);
    for (const auto &i_v : py::range(numModules)) {
        if (cluster_down_map.contains(i_v)) {
            const auto net = cluster_down_map[i_v];
            module_weight.push_back(weight_dict[net]);
        } else {
            const auto v2 = node_down_map[i_v];
            module_weight.push_back(hyprgraph.get_module_weight(v2));
        }
    }

    // if isinstance(hyprgraph.modules, range):
    //     node_up_map = [0 for _ in hyprgraph.modules]
    // elif isinstance(hyprgraph.modules, list):
    //     node_up_map = {}
    // else:
    //     raise NotImplementedError
    auto node_up_map = std::vector<node_t>(hyprgraph.modules.size());
    for (const auto &v : hyprgraph.modules) {
        node_up_map[v] = node_up_dict[v];
    }

    hgr2->node_up_map = std::move(node_up_map);
    hgr2->node_down_map = std::move(node_down_map);
    hgr2->cluster_down_map = std::move(cluster_down_map);
    hgr2->module_weight = std::move(module_weight);
    hgr2->parent = &hyprgraph;
    return hgr2;
}
