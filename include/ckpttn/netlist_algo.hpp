#include <algorithm>
// #include <range/v3/algorithm/any_of.hpp>
// #include <range/v3/algorithm/min_element.hpp>
#include <tuple>

/**
 * @brief minimum weighted vertex cover problem
 *
 *    This function solves minimum vertex cover problem
 *    using primal-dual paradigm:
 *
 * @tparam Gnl
 * @tparam C1
 * @tparam C2
 * @param[in] hyprgraph
 * @param[in] weight
 * @param[in,out] coverset in: pre-covered vetrices, out: sol'n set
 * @return C1::mapped_type
 */
template <typename Gnl, typename C1, typename C2>
auto min_vertex_cover(const Gnl &hyprgraph, const C1 &weight, C2 &coverset) ->
    typename C1::mapped_type {
    using T = typename C1::mapped_type;
    auto in_coverset = [&](const auto &v) { return coverset.contains(v); };
    auto total_dual_cost = T(0);
    static_assert(sizeof total_dual_cost >= 0, "maybe unused");
    auto total_primal_cost = T(0);
    auto gap = weight;
    for (const auto &net : hyprgraph.nets) {
        if (std::any_of(hyprgraph.gr[net].begin(), hyprgraph.gr[net].end(), in_coverset)) {
            continue;
        }

        auto min_vtx
            = *std::min_element(hyprgraph.gr[net].begin(), hyprgraph.gr[net].end(),
                                [&](const auto &v1, const auto &v2) { return gap[v1] < gap[v2]; });
        auto min_val = gap[min_vtx];
        coverset.insert(min_vtx);
        total_primal_cost += weight[min_vtx];
        total_dual_cost += min_val;
        for (const auto &u : hyprgraph.gr[net]) {
            gap[u] -= min_val;
        }
    }

    assert(total_dual_cost <= total_primal_cost);
    return total_primal_cost;
}

/**
 * @brief minimum weighted maximal matching problem
 *
 *    This function solves minimum maximal matching problem
 *    using primal-dual paradigm:
 *
 * @tparam Gnl
 * @tparam C1
 * @tparam C2
 * @param[in] hyprgraph
 * @param[in] weight
 * @param[in,out] matchset
 * @param[in,out] dep
 * @return C1::value_type
 */
template <typename Gnl, typename C1, typename C2>
auto min_maximal_matching(const Gnl &hyprgraph, const C1 &weight, C2 &matchset, C2 &dep) ->
    typename C1::mapped_type {
    auto cover = [&](const auto &net) {
        for (const auto &v : hyprgraph.gr[net]) {
            dep.insert(v);
        }
    };

    auto in_dep = [&](const auto &v) { return dep.contains(v); };

    // auto any_of_dep = [&](const auto& net) {
    //     return ranges::any_of(
    //         hyprgraph.gr[net], [&](const auto& v) { return dep.contains(v); });
    // };

    using T = typename C1::mapped_type;

    auto gap = weight;
    auto total_dual_cost = T(0);
    static_assert(sizeof total_dual_cost >= 0, "maybe unused");
    auto total_primal_cost = T(0);
    for (const auto &net : hyprgraph.nets) {
        if (std::any_of(hyprgraph.gr[net].begin(), hyprgraph.gr[net].end(), in_dep)) {
            continue;
        }
        if (matchset.contains(net)) {  // pre-define independant
            cover(net);
            continue;
        }
        auto min_val = gap[net];
        auto min_net = net;
        for (const auto &v : hyprgraph.gr[net]) {
            for (const auto &net2 : hyprgraph.gr[v]) {
                if (std::any_of(hyprgraph.gr[net2].begin(), hyprgraph.gr[net2].end(), in_dep)) {
                    continue;
                }
                if (min_val > gap[net2]) {
                    min_val = gap[net2];
                    min_net = net2;
                }
            }
        }
        cover(min_net);
        matchset.insert(min_net);
        total_primal_cost += weight[min_net];
        total_dual_cost += min_val;
        if (min_net != net) {
            gap[net] -= min_val;
            for (const auto &v : hyprgraph.gr[net]) {
                for (const auto &net2 : hyprgraph.gr[v]) {
                    gap[net2] -= min_val;
                }
            }
        }
    }
    // assert(total_dual_cost <= total_primal_cost);
    return total_primal_cost;
}
