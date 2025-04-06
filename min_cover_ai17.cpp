#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Node = size_t;

// MapAdapter implementation
template <typename T> class MapAdapter {
    std::vector<T> list;

  public:
    MapAdapter(std::vector<T> lst) : list(std::move(lst)) {}

    T& operator[](size_t index) { return list[index]; }
    const T& operator[](size_t index) const { return list[index]; }

    auto begin() { return list.begin(); }
    auto end() { return list.end(); }
    auto begin() const { return list.begin(); }
    auto end() const { return list.end(); }

    size_t size() const { return list.size(); }
};

// TinyGraph implementation
class TinyGraph {
    size_t num_nodes = 0;
    MapAdapter<std::unordered_map<Node, bool>> node_dict;
    MapAdapter<std::unordered_map<Node, bool>> adj_list;

  public:
    TinyGraph() = default;

    void init_nodes(size_t n) {
        num_nodes = n;
        node_dict = MapAdapter<std::unordered_map<Node, bool>>(
            std::vector<std::unordered_map<Node, bool>>(n));
        adj_list = MapAdapter<std::unordered_map<Node, bool>>(
            std::vector<std::unordered_map<Node, bool>>(n));
    }

    void add_edge(Node u, Node v) {
        adj_list[u][v] = true;
        adj_list[v][u] = true;
    }

    size_t degree(Node node) const { return adj_list[node].size(); }

    const std::unordered_map<Node, bool>& neighbors(Node node) const { return adj_list[node]; }

    size_t number_of_nodes() const { return num_nodes; }
};

// RepeatArray implementation
template <typename T> class RepeatArray {
    T value;
    size_t size;

  public:
    RepeatArray(T val, size_t sz) : value(std::move(val)), size(sz) {}

    T operator[](size_t) const { return value; }

    size_t get_size() const { return size; }

    T get(size_t) const { return value; }

    class iterator {
        T value;
        size_t count;

      public:
        iterator(T val, size_t cnt) : value(val), count(cnt) {}

        T operator*() const { return value; }
        iterator& operator++() {
            ++count;
            return *this;
        }
        bool operator!=(const iterator& other) const { return count != other.count; }
    };

    iterator begin() const { return iterator(value, 0); }
    iterator end() const { return iterator(value, size); }
};

// Netlist implementation
class Netlist {
  protected:
    TinyGraph ugraph;
    std::vector<Node> modules;
    std::vector<Node> nets;
    size_t num_modules;
    size_t num_nets;
    RepeatArray<size_t> module_weight;
    std::unordered_set<Node> module_fixed;
    size_t max_degree;

  public:
    Netlist(TinyGraph ug, std::vector<Node> mods, std::vector<Node> nts)
        : ugraph(std::move(ug)),
          modules(std::move(mods)),
          nets(std::move(nts)),
          num_modules(modules.size()),
          num_nets(nets.size()),
          module_weight(1, num_modules),
          max_degree(0) {
        for (const auto& m : modules) {
            max_degree = std::max(max_degree, ugraph.degree(m));
        }
    }

    size_t number_of_modules() const { return num_modules; }
    size_t number_of_nets() const { return num_nets; }
    size_t number_of_nodes() const { return ugraph.number_of_nodes(); }
    size_t number_of_pins() const { return ugraph.number_of_nodes(); }  // Simplified

    size_t get_max_degree() const { return max_degree; }
    size_t get_module_weight(Node v) const { return module_weight[v]; }
    size_t get_net_weight(Node) const { return 1; }

    auto begin() const { return modules.begin(); }
    auto end() const { return modules.end(); }
};

// HierNetlist implementation
class HierNetlist : public Netlist {
    std::shared_ptr<Netlist> parent;
    std::vector<Node> node_down_list;
    std::unordered_map<Node, size_t> net_weight;
    std::vector<Node> clusters;

  public:
    HierNetlist(TinyGraph ug, std::vector<Node> mods, std::vector<Node> nts)
        : Netlist(std::move(ug), std::move(mods), std::move(nts)),
          parent(std::make_shared<Netlist>(TinyGraph(), std::vector<Node>{}, std::vector<Node>{})) {
    }

    size_t get_degree(Node v) const {
        size_t sum = 0;
        for (const auto& [net, _] : ugraph.neighbors(v)) {
            sum += net_weight.count(net) ? net_weight.at(net) : 1;
        }
        return sum;
    }

    size_t get_max_degree() const {
        size_t max_deg = 0;
        for (Node v : modules) {
            max_deg = std::max(max_deg, get_degree(v));
        }
        return max_deg;
    }

    void projection_down(const std::vector<size_t>& part, std::vector<size_t>& part_down) const {
        size_t num_cells = node_down_list.size() - clusters.size();

        for (size_t v1 = 0; v1 < num_cells; ++v1) {
            part_down[node_down_list[v1]] = part[v1];
        }

        for (size_t i_v = 0; i_v < clusters.size(); ++i_v) {
            size_t p = part[num_cells + i_v];
            for (const auto& [v2, _] : parent->ugraph.neighbors(clusters[i_v])) {
                part_down[v2] = p;
            }
        }
    }

    void projection_up(const std::vector<size_t>& part, std::vector<size_t>& part_up) const {
        for (size_t v1 = 0; v1 < node_down_list.size(); ++v1) {
            part_up[v1] = part[node_down_list[v1]];
        }
    }

    size_t get_net_weight(Node net) const { return net_weight.count(net) ? net_weight.at(net) : 1; }
};

// min_maximal_matching implementation
std::pair<std::unordered_set<Node>, size_t> min_maximal_matching(
    const Netlist& hyprgraph, const std::unordered_map<Node, size_t>& weight,
    std::unordered_set<Node> matchset = {}, std::unordered_set<Node> dep = {}) {
    auto cover = [&hyprgraph](Node net, std::unordered_set<Node>& dep) {
        for (const auto& [vtx, _] : hyprgraph.ugraph.neighbors(net)) {
            dep.insert(vtx);
        }
    };

    auto any_of_dep = [&hyprgraph](Node net, const std::unordered_set<Node>& dep) {
        for (const auto& [vtx, _] : hyprgraph.ugraph.neighbors(net)) {
            if (dep.count(vtx)) return true;
        }
        return false;
    };

    size_t total_prml_cost = 0;
    size_t total_dual_cost = 0;

    std::unordered_map<Node, size_t> gap = weight;

    for (Node net : hyprgraph.nets) {
        if (any_of_dep(net, dep)) {
            continue;
        }
        if (matchset.count(net)) {
            continue;
        }

        size_t min_val = gap.count(net) ? gap[net] : 1;
        Node min_net = net;

        for (const auto& [vtx, _] : hyprgraph.ugraph.neighbors(net)) {
            for (const auto& [net2, _] : hyprgraph.ugraph.neighbors(vtx)) {
                if (any_of_dep(net2, dep)) {
                    continue;
                }
                size_t current = gap.count(net2) ? gap[net2] : 1;
                if (min_val > current) {
                    min_val = current;
                    min_net = net2;
                }
            }
        }

        cover(min_net, dep);
        matchset.insert(min_net);
        total_prml_cost += weight.count(min_net) ? weight.at(min_net) : 1;
        total_dual_cost += min_val;

        if (min_net == net) {
            continue;
        }

        gap[net] -= min_val;
        for (const auto& [vtx, _] : hyprgraph.ugraph.neighbors(net)) {
            for (const auto& [net2, _] : hyprgraph.ugraph.neighbors(vtx)) {
                gap[net2] -= min_val;
            }
        }
    }

    assert(total_dual_cost <= total_prml_cost);
    return {matchset, total_prml_cost};
}

// setup implementation
std::tuple<std::vector<Node>, std::vector<Node>, std::vector<Node>> setup(
    const Netlist& hyprgraph, const std::unordered_map<Node, size_t>& cluster_weight,
    const std::unordered_set<Node>& forbid = {}) {
    auto [s1, _] = min_maximal_matching(hyprgraph, cluster_weight, {}, forbid);

    std::unordered_set<Node> covered;
    std::vector<Node> nets;
    std::vector<Node> clusters;

    for (Node net : hyprgraph.nets) {
        if (s1.count(net)) {
            clusters.push_back(net);
            for (const auto& [v, _] : hyprgraph.ugraph.neighbors(net)) {
                covered.insert(v);
            }
        } else {
            nets.push_back(net);
        }
    }

    std::vector<Node> cell_list;
    for (Node v : hyprgraph) {
        if (!covered.count(v)) {
            cell_list.push_back(v);
        }
    }

    return {clusters, nets, cell_list};
}

// construct_graph implementation
TinyGraph construct_graph(const Netlist& hyprgraph, const std::vector<Node>& nets,
                          const std::vector<Node>& cell_list, const std::vector<Node>& clusters) {
    size_t num_modules = cell_list.size() + clusters.size();
    size_t num_cell = cell_list.size();

    std::unordered_map<Node, Node> node_up_map;
    for (size_t i_v = 0; i_v < clusters.size(); ++i_v) {
        Node net = clusters[i_v];
        for (const auto& [v, _] : hyprgraph.ugraph.neighbors(net)) {
            node_up_map[v] = i_v + num_cell;
        }
    }
    for (size_t i_v = 0; i_v < cell_list.size(); ++i_v) {
        node_up_map[cell_list[i_v]] = i_v;
    }

    TinyGraph ugraph;
    ugraph.init_nodes(num_modules + nets.size());

    for (size_t i_net = 0; i_net < nets.size(); ++i_net) {
        Node net = nets[i_net];
        for (const auto& [v, _] : hyprgraph.ugraph.neighbors(net)) {
            ugraph.add_edge(node_up_map[v], i_net + num_modules);
        }
    }

    return ugraph;
}

// purge_duplicate_nets implementation
std::pair<std::unordered_map<Node, size_t>, std::vector<Node>> purge_duplicate_nets(
    const Netlist& hyprgraph, const TinyGraph& ugraph, const std::vector<Node>& nets,
    size_t num_clusters, size_t num_modules) {
    size_t num_nets = nets.size();
    std::unordered_map<Node, size_t> net_weight;

    for (size_t i_net = 0; i_net < nets.size(); ++i_net) {
        size_t wt = hyprgraph.get_net_weight(nets[i_net]);
        if (wt != 1) {
            net_weight[num_modules + i_net] = wt;
        }
    }

    std::unordered_set<Node> removelist;
    for (Node cluster = num_modules - num_clusters; cluster < num_modules; ++cluster) {
        for (const auto& [net1, _] : ugraph.neighbors(cluster)) {
            assert(net1 >= num_modules);
            assert(net1 < num_modules + num_nets);

            if (ugraph.degree(net1) == 1) {
                removelist.insert(net1);
                continue;
            }

            for (const auto& [net2, _] : ugraph.neighbors(cluster)) {
                if (net2 == net1) continue;

                if (ugraph.degree(net1) != ugraph.degree(net2)) {
                    continue;
                }

                bool same = false;
                if (ugraph.degree(net1) <= 5) {
                    std::unordered_set<Node> set1, set2;
                    for (const auto& [v, _] : ugraph.neighbors(net1)) set1.insert(v);
                    for (const auto& [v, _] : ugraph.neighbors(net2)) set2.insert(v);
                    same = (set1 == set2);
                }

                if (same) {
                    removelist.insert(net2);
                    net_weight[net1] += net_weight.count(net2) ? net_weight[net2] : 1;
                }
            }
        }
    }

    std::cout << "removed " << removelist.size() << " nets" << std::endl;
    std::vector<Node> gr_nets;
    for (size_t i = num_modules; i < num_modules + nets.size(); ++i) {
        gr_nets.push_back(i);
    }

    std::vector<Node> updated_nets;
    for (Node net : gr_nets) {
        if (!removelist.count(net)) {
            updated_nets.push_back(net);
        }
    }

    return {net_weight, updated_nets};
}

// reconstruct_graph implementation
std::tuple<TinyGraph, std::unordered_map<Node, size_t>, size_t> reconstruct_graph(
    const Netlist& hyprgraph, const TinyGraph& ugraph, const std::vector<Node>& nets,
    size_t num_clusters, size_t num_modules) {
    auto [net_weight, updated_nets]
        = purge_duplicate_nets(hyprgraph, ugraph, nets, num_clusters, num_modules);

    size_t num_nets = updated_nets.size();
    TinyGraph gr2;
    gr2.init_nodes(num_modules + num_nets);

    for (size_t i_net = 0; i_net < updated_nets.size(); ++i_net) {
        Node net = updated_nets[i_net];
        for (const auto& [v, _] : ugraph.neighbors(net)) {
            assert(net >= num_modules);
            assert(net < num_modules + nets.size());
            gr2.add_edge(v, num_modules + i_net);
        }
    }

    std::unordered_map<Node, size_t> net_weight2;
    for (size_t i_net = 0; i_net < updated_nets.size(); ++i_net) {
        Node net = updated_nets[i_net];
        if (net_weight.count(net)) {
            net_weight2[i_net] = net_weight[net];
        }
    }

    return {gr2, net_weight2, num_nets};
}

// contract_subgraph implementation
std::pair<HierNetlist, std::vector<size_t>> contract_subgraph(
    const Netlist& hyprgraph, const std::vector<size_t>& module_weight,
    const std::unordered_set<Node>& forbid) {
    std::unordered_map<Node, size_t> cluster_weight;
    for (Node net : hyprgraph.nets) {
        size_t weight = 0;
        for (const auto& [v, _] : hyprgraph.ugraph.neighbors(net)) {
            weight += module_weight[v];
        }
        cluster_weight[net] = weight;
    }

    auto [clusters, nets, cell_list] = setup(hyprgraph, cluster_weight, forbid);
    TinyGraph ugraph = construct_graph(hyprgraph, nets, cell_list, clusters);

    size_t num_modules = cell_list.size() + clusters.size();
    size_t num_clusters = clusters.size();

    auto [gr2, net_weight2, num_nets]
        = reconstruct_graph(hyprgraph, ugraph, nets, num_clusters, num_modules);

    HierNetlist hgr2(gr2, std::vector<Node>(num_modules), std::vector<Node>(num_nets));

    std::vector<size_t> module_weight2(num_modules);
    size_t num_cells = num_modules - num_clusters;

    for (size_t v = 0; v < cell_list.size(); ++v) {
        module_weight2[v] = module_weight[cell_list[v]];
    }

    for (size_t i_v = 0; i_v < clusters.size(); ++i_v) {
        module_weight2[num_cells + i_v] = cluster_weight[clusters[i_v]];
    }

    std::vector<Node> node_down_list = cell_list;
    for (Node net : clusters) {
        node_down_list.push_back(hyprgraph.ugraph.neighbors(net).begin()->first);
    }

    hgr2.clusters = clusters;
    hgr2.node_down_list = node_down_list;
    hgr2.module_weight = module_weight2;
    hgr2.net_weight = net_weight2;
    hgr2.parent = std::make_shared<Netlist>(hyprgraph);

    return {hgr2, module_weight2};
}
