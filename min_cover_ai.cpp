#include <algorithm>
#include <set>
#include <unordered_map>

using namespace std;

struct HierNetlist;
struct Netlist;
struct TinyGraph;

using MutableMapping = unordered_map<string, int>;
using Set = set<string>;
using Tuple = pair<Set, int>;

/**
 * The `min_maximal_matching` function performs a minimum weighted maximal matching using a primal-dual
 * approximation algorithm.
 *
 * @param hgr The `hgr` parameter is an object representing a hypergraph. It likely contains
 * information about the vertices and edges of the hypergraph
 * @param weight The `weight` parameter is a mutable mapping that represents the weight of each net in
 * the hypergraph. It is used to determine the cost of each net in the matching
 * @param matchset The `matchset` parameter is a set that represents the initial matching. It contains
 * the nets (networks) that are already matched
 * @param dep The `dep` parameter is a set that keeps track of the vertices that have been covered by
 * the matching. It is initially set to an empty set, and is updated by the `cover` function. The
 * `cover` function takes a net as input and adds all the vertices connected to that net
 * @return The function `min_maximal_matching` returns a tuple containing the matchset (a set of
 * matched elements) and the total primal cost (an integer or float representing the total weight of
 * the matching).
 */
Tuple min_maximal_matching(
    HierNetlist* hgr,
    MutableMapping weight,
    Set* matchset = nullptr,
    Set* dep = nullptr,
) {
  // If the `matchset` parameter is not provided, create a new set
  if (matchset == nullptr) {
    matchset = new Set();
  }

  // If the `dep` parameter is not provided, create a new set
  if (dep == nullptr) {
    dep = new Set();
  }

  // The `cover` function takes a net as input and adds all the vertices connected to that net
  auto cover = [](string net) {
    for (auto vtx : hgr->gra[net]) {
      dep->insert(vtx);
    }
  };

  // The `any_of_dep` function returns `true` if the net is already covered by the matching
  auto any_of_dep = [](string net) {
    return any(vtx == net for vtx in hgr->gra[net]);
  };

  // Initialize the primal and dual costs
  int total_primal_cost = 0;
  int total_dual_cost = 0;

  // Create a copy of the weight mapping
  MutableMapping gap = weight;

  // Iterate over all the nets in the hypergraph
  for (string net : hgr->nets) {
    // If the net is already covered, skip it
    if (any_of_dep(net)) {
      continue;
    }

    // Initialize the minimum value
    int min_val = gap[net];
    string min_net = net;

    // Iterate over all the vertices connected to the net
    for (string vtx : hgr->gra[net]) {
      // Iterate over all the nets connected to the vertex
      for (string net2 : hgr->gra[vtx]) {
        // If the net is already covered, skip it
        if (any_of_dep(net2)) {
          continue;
        }

        // Update the minimum value if necessary
        if (min_val > gap[net2]) {
          min_val = gap[net2];
          min_net = net2;
        }
      }
    }

    // Add the net to the matching and update the costs
    cover(min_net);
    matchset->insert(min_net);
    total_primal_cost += weight[min_net];
    total_dual_cost += min_val;

    // Update the gap values
    if (min_net == net) {
      continue;
    }
    gap[net] -= min_val;
    for (string vtx : hgr->gra[net]) {
      for (string net2 : hgr->gra[vtx]) {
        // if net2 == net:
        //     continue
        gap[net2] -= min_val;
      }
    }
  }
  assert (total_dual_cost <= total_primal_cost);
  return {*matchset, total_primal_cost};
} 

