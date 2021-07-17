#include <algorithm>
// #include <range/v3/algorithm/any_of.hpp>
// #include <range/v3/algorithm/min_element.hpp>
#include <tuple>

/*!
 * @brief minimum weighted vertex cover problem
 *
 *    This function solves minimum vertex cover problem
 *    using primal-dual paradigm:
 *
 * @tparam Netlist
 * @tparam Container
 * @param[in] H
 * @param[in] weight
 * @param[in,out] cover initial: pre-covered vetrices
 * @return C1::value_type total cost
 */
template <typename Netlist, typename C1, typename C2>
auto min_vertex_cover(const Netlist& H, const C1& weight, C2& coverset) ->
    typename C1::mapped_type
{
    using T = typename C1::mapped_type;
    auto in_coverset = [&](const auto& v) { return coverset.contains(v); };
    [[maybe_unused]] auto total_dual_cost = T(0);
    auto total_primal_cost = T(0);
    auto gap = weight;
    for (const auto& net : H.nets)
    {
        if (std::any_of(H.G[net].begin(), H.G[net].end(), in_coverset))
        {
            continue;
        }

        auto min_vtx = *std::min_element(H.G[net].begin(), H.G[net].end(),
            [&](const auto& v1, const auto& v2) { return gap[v1] < gap[v2]; });
        auto min_val = gap[min_vtx];
        coverset.insert(min_vtx);
        total_primal_cost += weight[min_vtx];
        total_dual_cost += min_val;
        for (const auto& u : H.G[net])
        {
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
 * @tparam Netlist
 * @tparam C1
 * @tparam C2
 * @param H
 * @param weight
 * @param matchset
 * @param dep
 * @return C1::value_type
 */
template <typename Netlist, typename C1, typename C2>
auto min_maximal_matching(const Netlist& H, const C1& weight, C2&& matchset,
    C2&& dep) -> typename C1::mapped_type
{
    auto cover = [&](const auto& net)
    {
        for (const auto& v : H.G[net])
        {
            dep.insert(v);
        }
    };

    auto in_dep = [&](const auto& v) { return dep.contains(v); };

    // auto any_of_dep = [&](const auto& net) {
    //     return ranges::any_of(
    //         H.G[net], [&](const auto& v) { return dep.contains(v); });
    // };

    using T = typename C1::mapped_type;

    auto gap = weight;
    [[maybe_unused]] auto total_dual_cost = T(0);
    auto total_primal_cost = T(0);
    for (const auto& net : H.nets)
    {
        if (std::any_of(H.G[net].begin(), H.G[net].end(), in_dep))
        {
            continue;
        }
        if (matchset.contains(net))
        { // pre-define independant
            cover(net);
            continue;
        }
        auto min_val = gap[net];
        auto min_net = net;
        for (const auto& v : H.G[net])
        {
            for (const auto& net2 : H.G[v])
            {
                if (std::any_of(H.G[net2].begin(), H.G[net2].end(), in_dep))
                {
                    continue;
                }
                if (min_val > gap[net2])
                {
                    min_val = gap[net2];
                    min_net = net2;
                }
            }
        }
        cover(min_net);
        matchset.insert(min_net);
        total_primal_cost += weight[min_net];
        total_dual_cost += min_val;
        if (min_net != net)
        {
            gap[net] -= min_val;
            for (const auto& v : H.G[net])
            {
                for (const auto& net2 : H.G[v])
                {
                    gap[net2] -= min_val;
                }
            }
        }
    }
    // assert(total_dual_cost <= total_primal_cost);
    return total_primal_cost;
}
