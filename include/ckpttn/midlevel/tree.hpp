/**
 * @file tree.hpp
 * @brief Mid-level Gray code tree data structure
 *
 * Adapted from: Torsten Muetze, Jerri Nummenpalo (2018)
 * Original license: GPL v2+
 */

#ifndef CKPTTN_MIDLEVEL_TREE_HPP
#define CKPTTN_MIDLEVEL_TREE_HPP

#include <list>
#include <vector>

#include "vertex.hpp"

/**
 * @brief Tree data structure for mid-level Gray code algorithm
 *
 * Represents a rooted tree with children lists used in the middle-levels
 * Gray code algorithm. Supports tree rotations, flip operations, and
 * bitstring conversion for generating Hamiltonian cycles.
 */
class MidTree {
  public:
    /**
     * @brief Construct a MidTree from a MidVertex bitstring
     * @param[in] x The input MidVertex
     */
    explicit MidTree(const MidVertex& x);
    /// @brief Perform a flip-tree operation (tau transformation)
    bool flip_tree();
    /// @brief Rotate the tree to a canonical form
    void rotate();
    /**
     * @brief Convert the tree to a bitstring representation
     * @param[out] x Output bitstring array
     */
    void to_bitstring(int x[]) const;

  private:
    int num_vertices_;
    int root_;
    std::vector<std::list<int>> children_;
    std::vector<int> parent_;

    int deg(int u) const;
    int num_children(int u) const;
    int ith_child(int u, int i) const;
    bool is_tau_preimage() const;
    bool is_tau_image() const;
    void tau();
    void tau_inverse();
    void move_leaf(int leaf, int new_parent, int pos);
    void rotate_to_vertex(int u);
    void rotate_children();
    void rotate_children(int k);
    void root_canonically();
    void compute_center(int& c1, int& c2) const;
    bool is_flip_tree_tau();
    bool is_star() const;
    bool is_light_dumbbell() const;
    bool is_thin_leaf(int u) const;
    bool has_thin_leaf() const;
    int count_pending_edges(int u) const;
    void to_bitstring_rec(int x[], int u, int& pos) const;
    int min_string_rotation(int x[], int length);
};

#endif
