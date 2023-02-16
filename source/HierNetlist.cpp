// #include <__config>  // for std
#include <ckpttn/HierNetlist.hpp>
#include <py2cpp/range.hpp> // for _iterator, iterable_wrapper

#include "ckpttn/netlist.hpp" // for Netlist, Netlist<>::nodeview_t

using namespace std;

template <typename graph_t>
void HierNetlist<graph_t>::projection_up(gsl::span<const uint8_t> part,
                                         gsl::span<uint8_t> part_up) const {
  const auto &hgr = *this->parent;
  for (const auto &v : hgr) {
    part_up[this->node_up_map[v]] = part[v];
  }
}

template <typename graph_t>
void HierNetlist<graph_t>::projection_down(gsl::span<const uint8_t> part,
                                           gsl::span<uint8_t> part_down) const {
  const auto &hgr = *this->parent;
  for (const auto &v : this->modules) {
    if (this->cluster_down_map.contains(v)) {
      const auto net = this->cluster_down_map.at(v);
      for (const auto &v2 : hgr.gr[net]) {
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
