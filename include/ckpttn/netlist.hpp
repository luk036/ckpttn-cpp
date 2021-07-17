#pragma once

// import networkx as nx
// #include <algorithm>
#include <gsl/span>
#include <iterator>
#include <py2cpp/py2cpp.hpp>
#include <utility>
#include <vector>
#include <xnetwork/classes/graph.hpp>

// using node_t = int;

// struct PartInfo
// {
//     std::vector<std::uint8_t> part;
//     py::set<node_t> extern_nets;
// };

/*!
 * @brief Netlist
 *
 * Netlist is implemented by xn::Graph, which is a networkx-like graph.
 *
 */
template <typename graph_t>
struct Netlist
{
    using nodeview_t = typename graph_t::nodeview_t;
    using node_t = typename graph_t::node_t;
    using index_t = typename nodeview_t::key_type;
    // using graph_t = xn::Graph<graph_t>;

    graph_t G;
    nodeview_t modules;
    nodeview_t nets;
    size_t num_modules {};
    size_t num_nets {};
    size_t num_pads = 0U;
    size_t max_degree {};
    size_t max_net_degree {};
    // std::uint8_t cost_model = 0;
    std::vector<unsigned int> module_weight;
    bool has_fixed_modules {};
    py::set<node_t> module_fixed;

  public:
    /*!
     * @brief Construct a new Netlist object
     *
     * @param[in] G
     * @param[in] module_list
     * @param[in] net_list
     * @param[in] module_fixed
     */
    Netlist(graph_t G, const nodeview_t& modules, const nodeview_t& nets);

    /**
     * @brief Construct a new Netlist object
     *
     * @param[in] G
     * @param[in] num_modules
     * @param[in] num_nets
     */
    Netlist(graph_t G, uint32_t numModules, uint32_t numNets);

    auto begin() const
    {
        return this->modules.begin();
    }

    auto end() const
    {
        return this->modules.end();
    }

    /**
     * @brief Get the number of modules
     *
     * @return size_t
     */
    [[nodiscard]] auto number_of_modules() const -> size_t
    {
        return this->num_modules;
    }

    /*!
     * @brief Get the number of nets
     *
     * @return size_t
     */
    [[nodiscard]] auto number_of_nets() const -> size_t
    {
        return this->num_nets;
    }

    /*!
     * @brief Get the number of nodes
     *
     * @return size_t
     */
    [[nodiscard]] auto number_of_nodes() const -> size_t
    {
        return this->G.number_of_nodes();
    }

    // /*!
    //  * @brief
    //  *
    //  * @return index_t
    //  */
    // auto number_of_pins() const -> index_t { return
    // this->G.number_of_edges(); }

    /*!
     * @brief Get the max degree
     *
     * @return size_t
     */
    [[nodiscard]] auto get_max_degree() const -> size_t
    {
        return this->max_degree;
    }

    /*!
     * @brief Get the max net degree
     *
     * @return index_t
     */
    [[nodiscard]] auto get_max_net_degree() const -> size_t
    {
        return this->max_net_degree;
    }

    /**
     * @brief Get the module weight
     *
     * @param[in] v
     * @return int
     */
    auto get_module_weight(const node_t& v) const -> unsigned int
    {
        return this->module_weight.empty() ? 1U : this->module_weight[v];
    }

    /**
     * @brief Get the net weight
     *
     * @return int
     */
    auto get_net_weight(const node_t& /*net*/) const -> int
    {
        // return this->net_weight.is_empty() ? 1
        //                                 :
        //                                 this->net_weight[this->net_map[net]];
        return 1;
    }
};

/**
 * @brief Construct a new Netlist object
 *
 * @tparam nodeview_t
 * @tparam nodemap_t
 * @param[in] G
 * @param[in] modules
 * @param[in] nets
 */
template <typename graph_t>
Netlist<graph_t>::Netlist(
    graph_t G, const nodeview_t& modules, const nodeview_t& nets)
    : G {std::move(G)}
    , modules {modules}
    , nets {nets}
    , num_modules(modules.size())
    , num_nets(nets.size())
{
    this->has_fixed_modules = (!this->module_fixed.empty());

    // Some compilers does not accept py::range()->iterator as a forward
    // iterator auto deg_cmp = [this](const node_t& v, const node_t& w) ->
    // index_t {
    //     return this->G.degree(v) < this->G.degree(w);
    // };
    // const auto result1 =
    //     std::max_element(this->modules.begin(), this->modules.end(),
    //     deg_cmp);
    // this->max_degree = this->G.degree(*result1);
    // const auto result2 =
    //     std::max_element(this->nets.begin(), this->nets.end(), deg_cmp);
    // this->max_net_degree = this->G.degree(*result2);

    this->max_degree = 0U;
    for (const auto& v : this->modules)
    {
        if (this->max_degree < this->G.degree(v))
        {
            this->max_degree = this->G.degree(v);
        }
    }

    this->max_net_degree = 0U;
    for (const auto& net : this->nets)
    {
        if (this->max_net_degree < this->G.degree(net))
        {
            this->max_net_degree = this->G.degree(net);
        }
    }
}

template <typename graph_t>
Netlist<graph_t>::Netlist(graph_t G, uint32_t numModules, uint32_t numNets)
    : Netlist {std::move(G), py::range(numModules),
          py::range(numModules, numModules + numNets)}
{
}

// using RngIter = decltype(py::range(1));
using graph_t = xn::SimpleGraph;
using index_t = uint32_t;
using SimpleNetlist = Netlist<graph_t>;

template <typename Node>
struct MoveInfo
{
    Node net;
    Node v;
    std::uint8_t fromPart;
    std::uint8_t toPart;
};

template <typename Node>
struct MoveInfoV
{
    Node v;
    std::uint8_t fromPart;
    std::uint8_t toPart;
    // node_t v;
};

template <typename Node>
struct Snapshot
{
    py::set<Node> extern_nets;
    py::dict<index_t, std::uint8_t> extern_modules;
};
