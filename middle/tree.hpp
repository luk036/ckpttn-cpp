/*
 * Copyright (c) 2018 Torsten Muetze, Jerri Nummenpalo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TREE_HPP
#define TREE_HPP

#include "vertex.hpp"
#include <list>
#include <vector>

// A class to represent and manipulate an ordered rooted tree in doubly linked
// adjacency list representation.

class Tree {
  public:
    // Upon construction, we build the tree from the bitstring
    // representation of the given Vertex object. The first even number of
    // bits of the bitstring are interpreted as a lattice path that always
    // stays above the y-axis (=a Dyck path). Every 1-bit is interpreted as
    // attaching an edge/child to the current vertex of the tree, every
    // 0-bit is interpreted as going back to the parent vertex (without
    // adding any edges to the tree). For example, the bitstring
    // x=110111000010 corresponds to the following tree:
    //   /\
  //  /\
  //    \
  //     \
  //
    explicit Tree(const Vertex &x);

    // Decide if the tree is an end node of a directed edge in the
    // canonic spanning tree in the auxiliary graph defined in the paper
    // that defines a Hamilton cycle in the middle levels graph.
    // This function simultaneously applies tau() or tau_inverse()
    // to the tree.
    bool flip_tree();

    // rotate the tree by making the leftmost child of the root the new root
    void rotate();

    // Computes the bitstring representation of the tree.
    // The output array x should be allocated on the stack for speed
    // reasons.
    void to_bitstring(int x[]) const;

  private:
    int num_vertices_;
    int root_;
    // Doubly linked adjacency lists where this->children_[i] is a list of
    // the children of vertex i starting with the leftmost child and ending
    // with the rightmost child. The class assumes that the vertices of the
    // tree are numbers from 0 up to num_vertices_-1.
    std::vector<std::list<int>> children_;
    // The entry parent[i] is the parent of vertex i. The value of the
    // parent of this->root_ is undefined.
    std::vector<int> parent_;

    // #### auxiliary functions ####

    // the degree of a vertex
    int deg(int u) const;
    // the number of children of vertex u
    int num_children(int u) const;

    // Return the i-th child of vertex u when going through the children
    // from left to right. The value i is an integer in the interval [0,
    // num_children(u)[.
    int ith_child(int u, int i) const;

    // decide if the tree is a preimage of the mapping tau defined in the
    // paper
    bool is_tau_preimage() const;
    // decide if the tree is an image of the mapping tau defined in the
    // paper
    bool is_tau_image() const;

    // Apply the mapping tau defined in the paper.
    // The function assumes that this->is_tau_preimage == true.
    void tau();
    // Compute the inverse of the mapping tau defined in the paper.
    // The function assumes that this->is_tau_image == true.
    void tau_inverse();

    // Remove the given leaf of the tree and attach it to the vertex
    // new_parent at position pos in new_parent's list of children. The
    // position must be in the interval [0,...,num_children[new_parent]].
    void move_leaf(int leaf, int new_parent, int pos);

    // rotate until the given vertex u is the root
    void rotate_to_vertex(int u);
    // make the leftmost subtree of the root the rightmost subtree
    void rotate_children();
    // apply the previous rotation k times
    void rotate_children(int k);

    // Compute a canonical way to root the tree, i.e., the resulting
    // rooted tree will be the same regardless from which rooted
    // version the function is called (apart from vertex names, so
    // the resulting tree will have the same bitstring representation).
    void root_canonically();

    // Compute the center vertices of the tree.
    // If there is only one center, then it is returned in c1 and c2 is set
    // to -1. If there are two centers, then they are returned in c1 and c2.
    void compute_center(int &c1, int &c2) const;

    // Decide if the the edge (tree,tau(tree)) in the auxiliary graph
    // defined in the paper belongs to the canonic spanning tree that
    // defines a Hamilton cycle in the middle levels graph. See the paper
    // for a detailed definition of this function. The function assumes that
    // is_tau_preimage(tree) == true.
    bool is_flip_tree_tau(); // We promise that we do not modify the tree in
                             // this function. For speed reasons however we
                             // modify it temporarily and restore the state
                             // at the end of the function rather than
                             // making a copy of ourselves.

    // checks if the tree is a star, rooted at the center of a leaf
    bool is_star() const;
    // checks if tree has the form 1(10)^k0(10)^l with 1<=l<k
    bool is_light_dumbbell() const;
    // check if the vertex u is a thin leaf, i.e., whether it has degree 1
    // and its neighbor degree 2
    bool is_thin_leaf(int u) const;
    // checks if the tree has a thin leaf somewhere
    bool has_thin_leaf() const;
    // For the subtree rooted at the given vertex u, count the number
    // of pending edges starting at u from left to right until the first
    // non-pending edge is encountered.
    int count_pending_edges(int u) const;

    // Auxiliary recursive function to compute the bitstring representation.
    // The output array x should be allocated on the stack for speed
    // reasons.
    void to_bitstring_rec(int x[], int u, int &pos) const;

    // Booth's algorithm to compute lexicographically smallest
    // rotation of the given string/array. Return value is the
    // index where the lexicographically smallest string (viewed
    // cyclically) starts. The running time is linear in the length
    // of the input array.
    // The input array x should be allocated on the stack for speed reasons.
    // The reference to the original paper is:
    // [K. Booth, Lexicographically least circular substrings,
    //  Inf. Proc. Letters, 10 (4-5): 240–242]
    int min_string_rotation(int x[], int length);
};

#endif
