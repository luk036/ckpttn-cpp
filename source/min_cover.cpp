#include <ckpttn/netlist.hpp>       // for SimpleNetlist, index_t, Netlist
#include <ckpttn/netlist_algo.hpp>  // for min_maximal_matching
// #include <range/v3/all.hpp>
// #include <range/v3/core.hpp>
// #include <range/v3/numeric/accumulate.hpp>
// #include <range/v3/view/enumerate.hpp>
// #include <range/v3/view/remove_if.hpp>
// #include <range/v3/view/transform.hpp>
#include <cstdint>  // for uint32_t
// #include <__config>                    // for std
// #include <__hash_table>                // for operator!=, __hash_const_iterator
#include <ckpttn/HierNetlist.hpp>      // for SimpleHierNetlist, HierNetlist
#include <memory>                      // for unique_ptr, make_unique
#include <py2cpp/dict.hpp>             // for dict, dict<>::Base
#include <py2cpp/range.hpp>            // for _iterator, iterable_wrapper
#include <py2cpp/set.hpp>              // for set
#include <type_traits>                 // for move
#include <unordered_map>               // for __hash_map_iterator, operator!=
#include <utility>                     // for get
#include <vector>                      // for vector<>::iterator, vector
#include <xnetwork/classes/graph.hpp>  // for Graph, Graph<>::nodeview_t

using node_t = typename SimpleNetlist::node_t;
using namespace std;
// using namespace transrangers;

/**
 * @brief Create a contraction subgraph object
 *
 * @param[in] hgr
 * @param[in] DontSelect
 * @return unique_ptr<SimpleHierNetlist>
 * @todo simplify this function
 */
auto create_contraction_subgraph(const SimpleNetlist& hgr, const py::set<node_t>& DontSelect)
    -> unique_ptr<SimpleHierNetlist> {
    auto weight = py::dict<node_t, unsigned int>{};
    for (const auto& net : hgr.nets) {
        // weight[net] = accumulate(
        //     transform([&](const auto& v) { return hgr.get_module_weight(v); }, all(hgr.gr[net])),
        //     0U);
        auto sum = 0U;
        for (const auto& v : hgr.gr[net]) {
            sum += hgr.get_module_weight(v);
        }
        weight[net] = sum;
    }

    auto S = py::set<node_t>{};
    auto dep = DontSelect.copy();
    min_maximal_matching(hgr, weight, S, dep);

    auto module_up_map = py::dict<node_t, node_t>{};
    module_up_map.reserve(hgr.number_of_modules());
    for (const auto& v : hgr) {
        module_up_map[v] = v;
    }

    // auto cluster_map = py::dict<node_t, node_t> {};
    // cluster_map.reserve(S.size());
    auto node_up_dict = py::dict<node_t, index_t>{};
    auto net_up_map = py::dict<node_t, index_t>{};

    auto modules = vector<node_t>{};
    auto nets = vector<node_t>{};
    nets.reserve(hgr.nets.size() - S.size());

    {  // localize C and clusters
        auto C = py::set<node_t>{};
        auto clusters = vector<node_t>{};
        C.reserve(3 * S.size());  // TODO
        clusters.reserve(S.size());

        for (const auto& net : hgr.nets) {
            if (S.contains(net)) {
                // auto net_cur = hgr.gr[net].begin();
                // auto master = *net_cur;
                clusters.push_back(net);
                for (const auto& v : hgr.gr[net]) {
                    module_up_map[v] = net;
                    C.insert(v);
                }
                // cluster_map[master] = net;
            } else {
                nets.push_back(net);
            }
        }
        modules.reserve(hgr.modules.size() - C.size() + clusters.size());
        for (const auto& v : hgr) {
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
        for (const auto& v : modules) {
            module_map[v] = index_t(i_v);
            ++i_v;
        }

        // auto net_map = py::dict<node_t, index_t> {};
        net_up_map.reserve(numNets);
        auto i_net = index_t(0);
        for (const auto& net : nets) {
            net_up_map[net] = index_t(i_net) + numModules;
            ++i_net;
        }

        node_up_dict.reserve(hgr.number_of_modules());

        for (const auto& v : hgr) {
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
    for (const auto& v : hgr) {
        for (const auto& net : hgr.gr[v]) {
            if (S.contains(net)) {
                continue;
            }
            g.add_edge(node_up_dict[v], net_up_map[net]);
        }
    }
    // auto gr = py::grAdaptor<graph_t>(move(g));
    auto gr = move(g);

    auto hgr2 = make_unique<SimpleHierNetlist>(move(gr), py::range(numModules),
                                               py::range(numModules, numModules + numNets));

    auto node_down_map = vector<node_t>{};
    node_down_map.resize(numModules);
    // for (const auto& [v1, v2] : node_up_dict.items())
    for (const auto& keyvalue : node_up_dict.items()) {
        auto&& v1 = get<0>(keyvalue);
        auto&& v2 = get<1>(keyvalue);
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
    for (auto&& net : S) {
        for (auto&& v : hgr.gr[net]) {
            cluster_down_map[node_up_dict[v]] = net;
        }
    }

    auto module_weight = vector<unsigned int>{};
    module_weight.reserve(numModules);
    for (const auto& i_v : py::range(numModules)) {
        if (cluster_down_map.contains(i_v)) {
            const auto net = cluster_down_map[i_v];
            module_weight.push_back(weight[net]);
        } else {
            const auto v2 = node_down_map[i_v];
            module_weight.push_back(hgr.get_module_weight(v2));
        }
    }

    // if isinstance(hgr.modules, range):
    //     node_up_map = [0 for _ in hgr.modules]
    // elif isinstance(hgr.modules, list):
    //     node_up_map = {}
    // else:
    //     raise NotImplementedError
    auto node_up_map = vector<node_t>(hgr.modules.size());
    for (const auto& v : hgr.modules) {
        node_up_map[v] = node_up_dict[v];
    }

    hgr2->node_up_map = move(node_up_map);
    hgr2->node_down_map = move(node_down_map);
    hgr2->cluster_down_map = move(cluster_down_map);
    hgr2->module_weight = move(module_weight);
    hgr2->parent = &hgr;
    return hgr2;
}
