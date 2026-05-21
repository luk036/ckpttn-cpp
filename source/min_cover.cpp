#include <array>                       // for array
#include <ckpttn/HierNetlist.hpp>      // for SimpleHierNetlist, HierNetlist
#include <cstdint>                     // for uint32_t
#include <limits>                      // for numeric_limits
#include <memory>                      // for unique_ptr, make_unique
#include <netlistx/netlist.hpp>        // for SimpleNetlist, index_t, Netlist
#include <netlistx/netlist_algo.hpp>   // for min_maximal_matching
#include <py2cpp/dict.hpp>             // for dict
#include <py2cpp/range.hpp>            // for range
#include <py2cpp/set.hpp>              // for set
#include <unordered_map>               // for unordered_map
#include <utility>                     // for pair, move
#include <vector>                      // for vector
#include <xnetwork/classes/graph.hpp>  // for Graph, SimpleGraph

using node_t = SimpleNetlist::node_t;

static constexpr uint32_t LOW_PIN_NET_THRESHOLD = 5;
static constexpr uint32_t MINHASH_SIG_SIZE = 64;
static constexpr double MINHASH_SIMILARITY = 0.8;
static constexpr uint32_t MINHASH_MAX_DEGREE = 200;

using minhash_sig_t = std::array<uint64_t, MINHASH_SIG_SIZE>;

// Universal hash family: h_i(x) = hash_with_seed(x, seed_i)
static auto hash_with_seed(uint32_t x, uint64_t seed) noexcept -> uint64_t {
    auto h = seed;
    h ^= static_cast<uint64_t>(x) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
}

static auto minhash_signature(const graph_t& ugraph, uint32_t net) -> minhash_sig_t {
    auto sig = minhash_sig_t{};
    sig.fill(std::numeric_limits<uint64_t>::max());
    for (const auto& v : ugraph[net]) {
        for (auto i = 0U; i < MINHASH_SIG_SIZE; ++i) {
            auto h = hash_with_seed(static_cast<uint32_t>(v), static_cast<uint64_t>(i));
            if (h < sig[i]) {
                sig[i] = h;
            }
        }
    }
    return sig;
}

static auto jaccard_similarity(const minhash_sig_t& sig1, const minhash_sig_t& sig2) noexcept
    -> double {
    auto matches = 0U;
    for (auto i = 0U; i < MINHASH_SIG_SIZE; ++i) {
        if (sig1[i] == sig2[i]) {
            ++matches;
        }
    }
    return static_cast<double>(matches) / static_cast<double>(MINHASH_SIG_SIZE);
}

/**
 * @brief Setup function: find minimum maximal matching and create clusters, nets, cell_list.
 *
 * This function performs the initial setup for clustering by:
 * 1. Finding a minimum maximal matching in the hypergraph
 * 2. Creating clusters from the matched nets
 * 3. Separating remaining nets that weren't clustered
 * 4. Collecting cells that weren't included in any clusters
 *
 * @param[in] hyprgraph The input hypergraph
 * @param[in] cluster_weight Weight of each net for matching
 * @param[in,out] forbid Set of forbidden vertices (dependents), modified in-place
 * @return Tuple of {clusters, nets, cell_list}
 */
static auto setup(const SimpleNetlist& hyprgraph,
                  const py::dict<node_t, unsigned int>& cluster_weight, py::set<node_t>& forbid)
    -> std::tuple<std::vector<node_t>, std::vector<node_t>, std::vector<node_t>> {
    py::set<node_t> s1;
    min_maximal_matching(hyprgraph, cluster_weight, s1, forbid);

    py::set<node_t> covered;
    auto nets = std::vector<node_t>{};
    auto clusters = std::vector<node_t>{};

    for (const auto& net : hyprgraph.nets) {
        if (s1.contains(net)) {
            clusters.emplace_back(net);
            for (const auto& v : hyprgraph.gr[net]) {
                covered.insert(v);
            }
        } else {
            nets.emplace_back(net);
        }
    }

    auto cell_list = std::vector<node_t>{};
    for (const auto& v : hyprgraph) {
        if (!covered.contains(v)) {
            cell_list.emplace_back(v);
        }
    }

    return {std::move(clusters), std::move(nets), std::move(cell_list)};
}

/**
 * @brief Construct a bipartite graph from cell list, clusters, and nets.
 *
 * @param[in] hyprgraph The input hypergraph
 * @param[in] nets Nets that are not part of any cluster
 * @param[in] cell_list Individual cells not covered by any cluster
 * @param[in] clusters Cluster nets from the matching
 * @return Pair of {bipartite graph, node_up_map}
 */
static auto construct_graph(const SimpleNetlist& hyprgraph, const std::vector<node_t>& nets,
                            const std::vector<node_t>& cell_list,
                            const std::vector<node_t>& clusters)
    -> std::pair<graph_t, std::vector<node_t>> {
    auto num_cell = static_cast<uint32_t>(cell_list.size());
    auto num_clusters = static_cast<uint32_t>(clusters.size());
    auto num_modules = num_cell + num_clusters;
    auto num_nets = static_cast<uint32_t>(nets.size());

    auto node_up_map = std::vector<node_t>(hyprgraph.modules.size());

    // Cluster members map to indices [num_cell, num_modules)
    for (auto i_v = 0U; i_v < num_clusters; ++i_v) {
        auto net = clusters[i_v];
        for (const auto& v : hyprgraph.gr[net]) {
            node_up_map[v] = i_v + num_cell;
        }
    }

    for (auto i_v = 0U; i_v < num_cell; ++i_v) {
        node_up_map[cell_list[i_v]] = i_v;
    }

    auto g = graph_t(num_modules + num_nets);
    for (auto i_net = 0U; i_net < num_nets; ++i_net) {
        auto net = nets[i_net];
        for (const auto& v : hyprgraph.gr[net]) {
            g.add_edge(node_up_map[v], i_net + num_modules);
        }
    }

    return {std::move(g), std::move(node_up_map)};
}

/**
 * @brief Purge duplicate nets (nets connecting the same set of modules).
 *
 * Identifies and removes duplicate nets by:
 * 1. Checking for nets that connect exactly the same set of modules
 * 2. For low-pin nets (<= 5 connections), does exact set comparison
 * 3. Combining weights of duplicate nets into a single representative net
 *
 * @param[in] hyprgraph The input hypergraph
 * @param[in] ugraph The intermediate bipartite graph
 * @param[in] nets List of net IDs
 * @param[in] num_clusters Number of clusters
 * @param[in] num_modules Total number of modules (cells + clusters)
 * @return Pair of {net_weight map, updated list of net indices}
 */
static auto purge_duplicate_nets(const SimpleNetlist& hyprgraph, const graph_t& ugraph,
                                 const std::vector<node_t>& nets, uint32_t num_clusters,
                                 uint32_t num_modules)
    -> std::pair<py::dict<uint32_t, unsigned int>, std::vector<uint32_t>> {
    auto num_nets = static_cast<uint32_t>(nets.size());
    auto net_weight = py::dict<uint32_t, unsigned int>{};

    for (auto i_net = 0U; i_net < num_nets; ++i_net) {
        auto net = nets[i_net];
        auto wt = hyprgraph.get_net_weight(net);
        if (wt != 1) {
            net_weight[num_modules + i_net] = wt;
        }
    }

    auto removelist = py::set<uint32_t>{};
    std::unordered_map<uint32_t, minhash_sig_t> sig_cache;
    for (auto cluster = num_modules - num_clusters; cluster < num_modules; ++cluster) {
        for (const auto& net1 : ugraph[cluster]) {
            if (net1 < num_modules || net1 >= num_modules + num_nets) {
                continue;
            }

            if (ugraph.degree(net1) == 1)  // self-loop
            {
                removelist.insert(static_cast<uint32_t>(net1));
                continue;
            }

            for (const auto& net2 : ugraph[cluster]) {
                if (net2 == net1) {
                    continue;
                }
                if (ugraph.degree(net1) != ugraph.degree(net2)) {
                    continue;
                }

                auto same = false;
                auto deg = ugraph.degree(net1);
                if (deg <= LOW_PIN_NET_THRESHOLD) {
                    // Check both nets connect the same set of modules
                    const auto& set1 = ugraph[net1];
                    const auto& set2 = ugraph[net2];
                    if (set1.size() == set2.size()) {
                        same = true;
                        for (const auto& v : set1) {
                            if (!set2.contains(v)) {
                                same = false;
                                break;
                            }
                        }
                    }
                } else if (deg <= MINHASH_MAX_DEGREE) {
                    // MinHash pre-filter: skip exact comparison unless likely duplicate
                    auto it1 = sig_cache.find(static_cast<uint32_t>(net1));
                    if (it1 == sig_cache.end()) {
                        it1 = sig_cache
                                  .emplace(static_cast<uint32_t>(net1),
                                           minhash_signature(ugraph, static_cast<uint32_t>(net1)))
                                  .first;
                    }
                    auto it2 = sig_cache.find(static_cast<uint32_t>(net2));
                    if (it2 == sig_cache.end()) {
                        it2 = sig_cache
                                  .emplace(static_cast<uint32_t>(net2),
                                           minhash_signature(ugraph, static_cast<uint32_t>(net2)))
                                  .first;
                    }
                    auto sim = jaccard_similarity(it1->second, it2->second);
                    if (sim >= MINHASH_SIMILARITY) {
                        // Confirm with exact comparison to avoid false positives
                        const auto& set1 = ugraph[net1];
                        const auto& set2 = ugraph[net2];
                        if (set1.size() == set2.size()) {
                            same = true;
                            for (const auto& v : set1) {
                                if (!set2.contains(v)) {
                                    same = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (same) {
                    removelist.insert(static_cast<uint32_t>(net2));
                    net_weight[static_cast<uint32_t>(net1)]
                        = net_weight.get(static_cast<uint32_t>(net1), 1U)
                          + net_weight.get(static_cast<uint32_t>(net2), 1U);
                }
            }
        }
    }

    auto updated_nets = std::vector<uint32_t>{};
    for (auto i_net = num_modules; i_net < num_modules + num_nets; ++i_net) {
        if (!removelist.contains(i_net)) {
            updated_nets.push_back(i_net);
        }
    }

    return {std::move(net_weight), std::move(updated_nets)};
}

/**
 * @brief Reconstruct graph after purging duplicate nets.
 *
 * @param[in] hyprgraph The input hypergraph
 * @param[in] ugraph The intermediate bipartite graph
 * @param[in] nets List of net IDs
 * @param[in] num_clusters Number of clusters
 * @param[in] num_modules Total number of modules
 * @return Tuple of {reconstructed graph, net_weight map, number of nets}
 */
static auto reconstruct_graph(const SimpleNetlist& hyprgraph, const graph_t& ugraph,
                              const std::vector<node_t>& nets, uint32_t num_clusters,
                              uint32_t num_modules)
    -> std::tuple<graph_t, py::dict<uint32_t, unsigned int>, uint32_t> {
    auto [net_weight, updated_nets]
        = purge_duplicate_nets(hyprgraph, ugraph, nets, num_clusters, num_modules);

    auto num_nets = static_cast<uint32_t>(updated_nets.size());
    auto gr2 = graph_t(num_modules + num_nets);

    for (auto i_net = 0U; i_net < num_nets; ++i_net) {
        auto net = updated_nets[i_net];
        for (const auto& v : ugraph[net]) {
            gr2.add_edge(v, num_modules + i_net);
        }
    }

    auto net_weight2 = py::dict<uint32_t, unsigned int>{};
    for (auto i_net = 0U; i_net < num_nets; ++i_net) {
        auto net = updated_nets[i_net];
        if (net_weight.contains(net)) {
            net_weight2[i_net] = net_weight[net];
        }
    }

    return {std::move(gr2), std::move(net_weight2), num_nets};
}

/**
 * @brief Create a contracted subgraph from a hierarchical netlist.
 *
 * The main function that orchestrates the entire clustering process:
 * 1. Calculating initial cluster weights
 * 2. Setting up initial clusters and nets
 * 3. Constructing the intermediate graph
 * 4. Purging duplicates and reconstructing the final graph
 * 5. Creating the hierarchical netlist structure with updated weights
 *
 * @param[in] hyprgraph The input hypergraph
 * @param[in] dont_select Set of nets that should not be contracted
 * @return The contracted hierarchical netlist
 */
auto create_contracted_subgraph(const SimpleNetlist& hyprgraph, py::set<node_t> dont_select)
    -> std::unique_ptr<SimpleHierNetlist> {
    auto cluster_weight = py::dict<node_t, unsigned int>{};
    for (const auto& net : hyprgraph.nets) {
        auto sum = 0U;
        for (const auto& v : hyprgraph.gr[net]) {
            sum += hyprgraph.get_module_weight(v);
        }
        cluster_weight[net] = sum;
    }

    auto [clusters, nets, cell_list] = setup(hyprgraph, cluster_weight, dont_select);

    auto [ugraph, node_up_map] = construct_graph(hyprgraph, nets, cell_list, clusters);

    auto num_modules = static_cast<uint32_t>(cell_list.size() + clusters.size());
    auto num_clusters = static_cast<uint32_t>(clusters.size());

    auto [gr2, net_weight2, num_nets]
        = reconstruct_graph(hyprgraph, ugraph, nets, num_clusters, num_modules);

    auto hgr2 = std::make_unique<SimpleHierNetlist>(std::move(gr2), py::range(num_modules),
                                                    py::range(num_modules, num_modules + num_nets));

    auto module_weight2 = std::vector<unsigned int>(num_modules, 0U);
    auto num_cells = num_modules - num_clusters;

    for (auto v = 0U; v < num_cells; ++v) {
        module_weight2[v] = hyprgraph.get_module_weight(cell_list[v]);
    }
    for (auto i_v = 0U; i_v < num_clusters; ++i_v) {
        module_weight2[num_cells + i_v] = cluster_weight[clusters[i_v]];
    }

    auto node_down_map = std::vector<node_t>(cell_list.begin(), cell_list.end());
    for (const auto& net : clusters) {
        node_down_map.emplace_back(*hyprgraph.gr[net].begin());
    }

    auto cluster_down_map = py::dict<uint32_t, node_t>{};
    for (auto i_v = 0U; i_v < num_clusters; ++i_v) {
        auto net = clusters[i_v];
        for (const auto& v : hyprgraph.gr[net]) {
            cluster_down_map[node_up_map[v]] = net;
        }
    }

    hgr2->node_up_map = std::move(node_up_map);
    hgr2->node_down_map = std::move(node_down_map);
    hgr2->cluster_down_map = std::move(cluster_down_map);
    hgr2->module_weight = std::move(module_weight2);

    if (!net_weight2.empty()) {
        hgr2->net_weight.set_start(0);
        hgr2->net_weight.resize(num_nets, 1U);
        for (auto i = 0U; i < num_nets; ++i) {
            if (net_weight2.contains(i)) {
                hgr2->net_weight[i] = net_weight2[i];
            }
        }
    }

    hgr2->parent = &hyprgraph;
    return hgr2;
}
