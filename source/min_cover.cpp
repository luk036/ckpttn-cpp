#include <ckpttn/bpqueue.hpp>
#include <ckpttn/netlist.hpp>
#include <ckpttn/netlist_algo.hpp>
#include <memory>
#include <py2cpp/py2cpp.hpp>
// #include <range/v3/all.hpp>
// #include <range/v3/core.hpp>
// #include <range/v3/numeric/accumulate.hpp>
// #include <range/v3/view/enumerate.hpp>
// #include <range/v3/view/remove_if.hpp>
// #include <range/v3/view/transform.hpp>
#include <ckpttn/HierNetlist.hpp>
// #include <transrangers.hpp>
#include <tuple>
#include <vector>

using node_t = typename SimpleNetlist::node_t;
// using namespace transrangers;

/**
 * @brief Create a contraction subgraph object
 *
 * @param[in] H
 * @param[in] DontSelect
 * @return std::unique_ptr<SimpleHierNetlist>
 * @todo simplify this function
 */
auto create_contraction_subgraph(const SimpleNetlist& H, const py::set<node_t>& DontSelect)
    -> std::unique_ptr<SimpleHierNetlist> {
    auto weight = py::dict<node_t, unsigned int>{};
    for (const auto& net : H.nets) {
        // weight[net] = accumulate(
        //     transform([&](const auto& v) { return H.get_module_weight(v); }, all(H.G[net])), 0U);
        auto sum = 0U;
        for (const auto& v : H.G[net]) {
            sum += H.get_module_weight(v);
        }
        weight[net] = sum;
    }

    auto S = py::set<node_t>{};
    auto dep = DontSelect.copy();
    min_maximal_matching(H, weight, S, dep);

    auto module_up_map = py::dict<node_t, node_t>{};
    module_up_map.reserve(H.number_of_modules());
    for (const auto& v : H) {
        module_up_map[v] = v;
    }

    // auto cluster_map = py::dict<node_t, node_t> {};
    // cluster_map.reserve(S.size());
    auto node_up_dict = py::dict<node_t, index_t>{};
    auto net_up_map = py::dict<node_t, index_t>{};

    auto modules = std::vector<node_t>{};
    auto nets = std::vector<node_t>{};
    nets.reserve(H.nets.size() - S.size());

    {  // localize C and clusters
        auto C = py::set<node_t>{};
        auto clusters = std::vector<node_t>{};
        C.reserve(3 * S.size());  // ???
        clusters.reserve(S.size());

        for (const auto& net : H.nets) {
            if (S.contains(net)) {
                // auto netCur = H.G[net].begin();
                // auto master = *netCur;
                clusters.push_back(net);
                for (const auto& v : H.G[net]) {
                    module_up_map[v] = net;
                    C.insert(v);
                }
                // cluster_map[master] = net;
            } else {
                nets.push_back(net);
            }
        }
        modules.reserve(H.modules.size() - C.size() + clusters.size());
        for (const auto& v : H) {
            if (C.contains(v)) {
                continue;
            }
            modules.push_back(v);
        }
        modules.insert(modules.end(), clusters.begin(), clusters.end());
    }
    // auto nodes = std::vector<node_t>{};
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

        node_up_dict.reserve(H.number_of_modules());

        for (const auto& v : H) {
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
    // G.add_nodes_from(nodes);
    for (const auto& v : H) {
        for (const auto& net : H.G[v]) {
            if (S.contains(net)) {
                continue;
            }
            g.add_edge(node_up_dict[v], net_up_map[net]);
        }
    }
    // auto G = py::grAdaptor<graph_t>(std::move(g));
    auto G = std::move(g);

    auto H2 = std::make_unique<SimpleHierNetlist>(std::move(G), py::range(numModules),
                                                  py::range(numModules, numModules + numNets));

    auto node_down_map = std::vector<node_t>{};
    node_down_map.resize(numModules);
    // for (const auto& [v1, v2] : node_up_dict.items())
    for (const auto& keyvalue : node_up_dict.items()) {
        auto&& v1 = std::get<0>(keyvalue);
        auto&& v2 = std::get<1>(keyvalue);
        node_down_map[v2] = v1;
    }
    auto cluster_down_map = py::dict<index_t, node_t>{};
    // cluster_down_map.reserve(cluster_map.size()); // ???
    // // for (const auto& [v, net] : cluster_map.items())
    // for (const auto& keyvalue : cluster_map.items())
    // {
    //     auto&& v = std::get<0>(keyvalue);
    //     auto&& net = std::get<1>(keyvalue);
    //     cluster_down_map[node_up_dict[v]] = net;
    // }
    for (auto&& net : S) {
        for (auto&& v : H.G[net]) {
            cluster_down_map[node_up_dict[v]] = net;
        }
    }

    auto module_weight = std::vector<unsigned int>{};
    module_weight.reserve(numModules);
    for (const auto& i_v : py::range(numModules)) {
        if (cluster_down_map.contains(i_v)) {
            const auto net = cluster_down_map[i_v];
            module_weight.push_back(weight[net]);
        } else {
            const auto v2 = node_down_map[i_v];
            module_weight.push_back(H.get_module_weight(v2));
        }
    }

    // if isinstance(H.modules, range):
    //     node_up_map = [0 for _ in H.modules]
    // elif isinstance(H.modules, list):
    //     node_up_map = {}
    // else:
    //     raise NotImplementedError
    auto node_up_map = std::vector<node_t>(H.modules.size());
    for (const auto& v : H.modules) {
        node_up_map[v] = node_up_dict[v];
    }

    H2->node_up_map = std::move(node_up_map);
    H2->node_down_map = std::move(node_down_map);
    H2->cluster_down_map = std::move(cluster_down_map);
    H2->module_weight = std::move(module_weight);
    H2->parent = &H;
    return H2;
}
