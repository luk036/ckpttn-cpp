#include <algorithm>
#include <ckpttn/HierNetlist.hpp>  // for SimpleHierNetlist, HierNetlist
#include <cstdint>                 // for uint32_t
#include <iostream>
#include <memory>                     // for unique_ptr, make_unique
#include <netlistx/netlist.hpp>       // for SimpleNetlist, index_t, Netlist
#include <netlistx/netlist_algo.hpp>  // for min_maximal_matching
#include <py2cpp/dict.hpp>            // for dict, dict<>::Base
#include <py2cpp/range.hpp>           // for _iterator, iterable_wrapper
#include <py2cpp/set.hpp>             // for set
#include <transrangers.hpp>           // for accumlate, transform, all
#include <type_traits>                // for move
#include <unordered_map>              // for __hash_map_iterator, operator!=
#include <unordered_set>
#include <utility>  // for get
#include <vector>   // for vector<>::iterator, vector
#include <vector>
#include <xnetwork/classes/graph.hpp>  // for Graph, Graph<>::nodeview_t

using namespace std;
using node_t = typename SimpleNetlist::node_t;

using MutableMapping = unordered_map<int, int>;
using Set = unordered_set<int>;
using Tuple = pair<Set, int>;
using Union = Tuple;

Tuple min_maximal_matching(Hypergraph& hyprgraph, MutableMapping& weight, Set* matchset = nullptr,
                           Set* dep = nullptr) {
    if (matchset == nullptr) {
        matchset = new Set();
    }
    if (dep == nullptr) {
        dep = new Set();
    }

    auto cover = [&](int net) {
        for (int vtx : hyprgraph.gra[net]) {
            dep->insert(vtx);
        }
    };

    auto any_of_dep = [&](int net) {
        return any_of(dep->begin(), dep->end(), [&](int vtx) {
            return find(hyprgraph.gra[net].begin(), hyprgraph.gra[net].end(), vtx)
                   != hyprgraph.gra[net].end();
        });
    };

    int total_primal_cost = 0;
    int total_dual_cost = 0;

    MutableMapping gap = weight;
    for (int net : hyprgraph.nets) {
        if (any_of_dep(net)) {
            continue;
        }
        if (matchset->count(net) > 0) {
            continue;
        }
        int min_val = gap[net];
        int min_net = net;
        for (int vtx : hyprgraph.gra[net]) {
            for (int net2 : hyprgraph.gra[vtx]) {
                if (any_of_dep(net2)) {
                    continue;
                }
                if (min_val > gap[net2]) {
                    min_val = gap[net2];
                    min_net = net2;
                }
            }
        }
        cover(min_net);
        matchset->insert(min_net);
        total_primal_cost += weight[min_net];
        total_dual_cost += min_val;
        if (min_net == net) {
            continue;
        }
        gap[net] -= min_val;
        for (int vtx : hyprgraph.gra[net]) {
            for (int net2 : hyprgraph.gra[vtx]) {
                gap[net2] -= min_val;
            }
        }
    }

    delete matchset;
    delete dep;

    return make_pair(*matchset, total_primal_cost);
}

Tuple setup(Hypergraph& hyprgraph, MutableMapping& cluster_weight, Set& forbid) {
    Set* matchset = new Set();
    Set* dep = new Set();

    Tuple result = min_maximal_matching(hyprgraph, cluster_weight, matchset, dep);

    Set& s1 = result.first;
    Set covered;
    vector<int> nets;
    vector<int> clusters;
    for (int net : hyprgraph.nets) {
        if (s1.count(net) > 0) {
            clusters.push_back(net);
            for (int v : hyprgraph.gra[net]) {
                covered.insert(v);
            }
        } else {
            nets.push_back(net);
        }
    }

    vector<int> cell_list;
    for (int v : hyprgraph.gra) {
        if (covered.count(v) == 0) {
            cell_list.push_back(v);
        }
    }

    delete matchset;
    delete dep;

    return make_pair(clusters, nets, cell_list);
}

unordered_map<int, vector<int>> construct_graph(Hypergraph& hyprgraph, vector<int>& nets,
                                                vector<int>& cell_list, vector<int>& clusters) {
    int num_modules = cell_list.size() + clusters.size();
    unordered_map<int, int> node_up_map;
    for (int i_v = 0; i_v < clusters.size(); i_v++) {
        int net = clusters[i_v];
        for (int v : hyprgraph.gra[net]) {
            node_up_map[v] = i_v + cell_list.size();
        }
    }
    for (int i_v = 0; i_v < cell_list.size(); i_v++) {
        int v = cell_list[i_v];
        node_up_map[v] = i_v;
    }

    unordered_map<int, vector<int>> gra;
    for (int i_net = 0; i_net < nets.size(); i_net++) {
        int net = nets[i_net];
        for (int v : hyprgraph.gra[net]) {
            gra[node_up_map[v]].push_back(i_net + num_modules);
        }
    }

    return gra;
}

Tuple purge_duplicate_nets(Hypergraph& hyprgraph, unordered_map<int, vector<int>>& gra,
                           vector<int>& nets, int num_clusters, int num_modules) {
    unordered_map<int, int> net_weight;
    for (int i_net = 0; i_net < nets.size(); i_net++) {
        int net = nets[i_net];
        int wt = hyprgraph.get_net_weight(net);
        if (wt != 1) {
            net_weight[num_modules + i_net] = wt;
        }
    }

    unordered_set<int> removelist;
    for (int cluster = num_modules - num_clusters; cluster < num_modules; cluster++) {
        for (int net1 : gra[cluster]) {
            if (gra[cluster].size() == 1) {
                removelist.insert(net1);
                continue;
            }
            for (int net2 : gra[cluster]) {
                if (gra[cluster].size() != gra[net2].size()) {
                    continue;
                }
                bool same = false;
                if (gra[cluster].size() <= 5) {
                    unordered_set<int> set1(gra[net1].begin(), gra[net1].end());
                    unordered_set<int> set2(gra[net2].begin(), gra[net2].end());
                    if (set1 == set2) {
                        same = true;
                    }
                }
                if (same) {
                    removelist.insert(net2);
                    net_weight[net1] = net_weight[net1] + net_weight[net2];
                }
            }
        }
    }

    vector<int> updated_nets;
    for (int net : nets) {
        if (removelist.count(net) == 0) {
            updated_nets.push_back(net);
        }
    }

    return make_pair(net_weight, updated_nets);
}

Tuple reconstruct_graph(Hypergraph& hyprgraph, unordered_map<int, vector<int>>& gra,
                        vector<int>& nets, int num_clusters, int num_modules) {
    Tuple result = purge_duplicate_nets(hyprgraph, gra, nets, num_clusters, num_modules);
    unordered_map<int, int> net_weight = result.first;
    vector<int> updated_nets = result.second;

    int num_nets = updated_nets.size();
    unordered_map<int, vector<int>> gr2;
    for (int i_net = 0; i_net < updated_nets.size(); i_net++) {
        int net = updated_nets[i_net];
        for (int v : gra[net]) {
            gr2[v].push_back(num_modules + i_net);
        }
    }

    return make_pair(gr2, net_weight, num_nets);
}

Tuple contract_subgraph(Hypergraph& hyprgraph, MutableMapping& module_weight, Set& forbid) {
    MutableMapping cluster_weight;
    for (int net : hyprgraph.nets) {
        int weight = 0;
        for (int v : hyprgraph.gra[net]) {
            weight += module_weight[v];
        }
        cluster_weight[net] = weight;
    }

    Tuple result = setup(hyprgraph, cluster_weight, forbid);
    vector<int> clusters = result.first;
    vector<int> nets = result.second;
    vector<int> cell_list = result.third;

    unordered_map<int, vector<int>> gra = construct_graph(hyprgraph, nets, cell_list, clusters);

    int num_modules = cell_list.size() + clusters.size();
    int num_clusters = clusters.size();

    Tuple result2 = reconstruct_graph(hyprgraph, gra, nets, num_clusters, num_modules);
    unordered_map<int, vector<int>> gr2 = result2.first;
    unordered_map<int, int> net_weight2 = result2.second;
    int num_nets = result2.third;

    return make_pair(gr2, net_weight2, num_nets);
}
