// #include <__config>  // for std
#include <ckpttn/HierNetlist.hpp>
#include <netlistx/netlist.hpp>  // for Netlist, Netlist<>::nodeview_t
#include <py2cpp/range.hpp>      // for _iterator, iterable_wrapper

using namespace std;

template <typename graph_t>
void HierNetlist<graph_t>::projection_up(std::span<const uint8_t> part,
                                         std::span<uint8_t> part_up) const {
    const auto &hyprgraph = *this->parent;
    for (const auto &v : hyprgraph) {
        part_up[this->node_up_map[v]] = part[v];
    }
}

template <typename graph_t>
void HierNetlist<graph_t>::projection_down(std::span<const uint8_t> part,
                                           std::span<uint8_t> part_down) const {
    const auto &hyprgraph = *this->parent;
    for (const auto &v : this->modules) {
        if (this->cluster_down_map.contains(v)) {
            const auto net = this->cluster_down_map.at(v);
            for (const auto &v2 : hyprgraph.gr[net]) {
                part_down[v2] = part[v];
            }
        } else {
            const auto v2 = this->node_down_map[v];
            part_down[v2] = part[v];
        }
    }
    // if (extern_nets.empty()) {
    //     return;
    // }
    // extern_nets_down.clear();
    // extern_nets_down.reserve(extern_nets.size());
    // for (auto net : extern_nets) {
    //     extern_nets_down.insert(this->node_down_map[net]);
    // }
}

template class HierNetlist<xnetwork::SimpleGraph>;
