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

#include "tree.hpp"
#include "vertex.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <list>
#include <vector>

Tree::Tree(const Vertex &x) {
    std::vector<int> xv = x.get_bits(); // extract bitstring representation
    assert(xv.size() % 2 == 1);

    this->root_ = 0;
    this->num_vertices_ = (xv.size() - 1) / 2 + 1;
    this->children_.resize(this->num_vertices_);
    this->parent_.resize(num_vertices_, 0);

    int u = this->root_; // the current vertex
    int n = 1;     // the number of vertices created so far
    int height = 0;
    for (int i = 0; i < xv.size() - 1; ++i) {
        if (x[i] == 1) {
            // create a new child vertex and add an edge from the
            // current vertex leading to the new child
            this->children_[u].push_back(n);
            this->parent_[n] = u;
            u = n;
            ++n;
        } else {
            // move back to the parent of the current vertex
            u = this->parent_[u];
        }
        height += (2 * x[i] - 1);
        assert(height >= 0);
    }
    assert(n == this->num_vertices_);
}

int Tree::deg(int u) const {
    assert((0 <= u) && (u < this->num_vertices_));
    if (u == this->root_) {
        return this->children_[u].size();
    } else {
        return this->children_[u].size() + 1;
    }
}

int Tree::num_children(int u) const {
    assert((0 <= u) && (u < this->num_vertices_));
    return this->children_[u].size();
}

int Tree::ith_child(int u, int i) const {
    assert((0 <= u) && (u < this->num_vertices_));
    assert((0 <= i) && (i < num_children(u)));
    // move to i-th entry from the beginning
    std::list<int>::const_iterator it = this->children_[u].begin();
    std::advance(it, i);
    return *it;
}

// The tree has to look like this (x and y are arbitrary subtrees):
//   /x
//  /y
bool Tree::is_tau_preimage() const {
    if (this->num_vertices_ < 3) {
        return false;
    }
    // u is the leftmost child of the root
    const int u = ith_child(root_, 0);
    if (num_children(u) == 0) {
        return false;
    }
    // v is the leftmost child of u
    const int v = ith_child(u, 0);
    if (num_children(v) != 0) {
        return false;
    }
    return true;
}

// The tree has to look like this (x and y are arbitrary subtrees):
//   /|x
//    y
bool Tree::is_tau_image() const {
    if ((this->num_vertices_ < 3) || (num_children(this->root_) < 2) ||
        (num_children(ith_child(root_, 0)) > 0)) {
        return false;
    }
    return true;
}

void Tree::tau() {
    assert(is_tau_preimage());
    const int u = ith_child(root_, 0);
    const int v = ith_child(u, 0);
    move_leaf(v, root_, 0);
}

void Tree::tau_inverse() {
    assert(is_tau_image());
    const int v = ith_child(root_, 0);
    const int u = ith_child(root_, 1);
    move_leaf(v, u, 0);
}

void Tree::move_leaf(int leaf, int new_parent, int pos) {
    assert((0 <= leaf) && (leaf < this->num_vertices_));
    assert((0 <= new_parent) && (new_parent < this->num_vertices_));
    assert((0 <= pos) && (pos <= this->children_[new_parent].size()));
    assert(num_children(leaf) == 0);
    const int old_parent = this->parent_[leaf];
    // search through the children of the current parent
    for (std::list<int>::iterator it = this->children_[old_parent].begin();
         it != this->children_[old_parent].end(); ++it) {
        if (*it == leaf) { // remove this child
            this->children_[old_parent].erase(it);
            break;
        }
    }
    // add the leaf below the new parent vertex
    std::list<int>::iterator it = this->children_[new_parent].begin();
    std::advance(it, pos);
    this->children_[new_parent].insert(it, leaf);
    this->parent_[leaf] = new_parent;
}

void Tree::rotate() {
    assert(this->num_vertices_ >= 2);
    const int u = ith_child(root_, 0);
    this->parent_[root_] = u;
    // move first entry from root_'s list of children to end of u's list of
    // children (this is much faster than pop_front() and push_back())
    this->children_[u].splice(this->children_[u].end(), this->children_[root_],
                              this->children_[root_].begin());
    this->children_[u].back() = root_;
    this->root_ = u;
}

void Tree::rotate_to_vertex(int u) {
    while (this->root_ != u) {
        rotate();
    }
}

void Tree::rotate_children() { rotate_children(1); }

void Tree::rotate_children(int k) {
    std::list<int>::iterator it = this->children_[root_].begin();
    std::advance(it, k);
    std::rotate(this->children_[root_].begin(), it,
                this->children_[root_].end());
}

bool Tree::flip_tree() {
    if (is_tau_preimage() && is_flip_tree_tau()) {
        tau();
        return true;
    } else if (is_tau_image()) {
        tau_inverse();
        if (is_flip_tree_tau()) {
            return true;
        }
        tau(); // undo tau^{-1}
    }
    return false;
}

void Tree::root_canonically() {
    int c1, c2; // center vertices
    compute_center(c1, c2);
    if (c2 != -1) { // centers are different
        // compute bitstring representation x1 when rooting
        // the tree at c1 such that c2 is the leftmost child
        const int num_bits = 2 * (this->num_vertices_ - 1);
        int x1[num_bits];
        int x2[num_bits];
        rotate_to_vertex(c1);
        while (ith_child(root_, 0) != c2) {
            rotate_children();
        }
        to_bitstring(x1);

        // compute bitstring representation x2 when rooting
        // the tree at c2 such that c1 is the leftmost child
        rotate();
        rotate_children(num_children(this->root_) - 1);
        assert((this->root_ == c2) && (ith_child(root_, 0) == c1));
        to_bitstring(x2);

        // The canonical rooting of the tree is the one corresponding
        // to the lexicographically smallest bitstring representation
        if (bitstrings_less_than(x1, x2, num_bits)) {
            rotate();
            rotate_children(num_children(this->root_) - 1);
            assert((this->root_ == c1) && (ith_child(root_, 0) == c2));
        }
    } else { // centers are the same
        // root at the center and compute bitstring representation
        rotate_to_vertex(c1);
        const int num_bits = 2 * (this->num_vertices_ - 1);
        int x[num_bits];
        to_bitstring(x);

        // compute segments of the bitstring representation
        // belonging to the different subtrees
        int subtree_count[num_bits];
        int c = 0;
        int depth = 0;
        for (int i = 0; i < num_bits; ++i) {
            if (x[i] == 1) {
                ++depth;
            } else { // x[i] == 0
                --depth;
            }
            subtree_count[i] = c;
            if (depth == 0) {
                ++c;
            }
        }
        assert(c == num_children(this->root_));

        // Compute the lexicographically smallest rotation of the given
        // string. Note that this function returns slightly different
        // results than the function used in the paper which adds
        // additional -1s to between subtrees. We do not add these -1s,
        // but still obtain a canonically rooted tree.
        const int k = min_string_rotation(x, num_bits);

        // rotate children accordingly
        rotate_children(subtree_count[k]);
    }
}

void Tree::compute_center(int &c1, int &c2) const {
    // set vertex degrees and store leaves
    std::vector<int> degs(num_vertices_, 0);
    std::vector<int> leaves(num_vertices_,
                            0); // for sure this many entries will be enough
    int num_leaves = 0;
    for (int i = 0; i < num_vertices_; ++i) {
        degs[i] = deg(i);
        if (degs[i] == 1) {
            leaves[num_leaves++] = i;
        }
    }

    int num_vertices_remaining = num_vertices_;
    int num_new_leaves = 0;
    // cut away leaves in rounds until only 1 or 2 vertices are left,
    // they form the center vertices
    while (num_vertices_remaining > 2) {
        // remove leaves
        for (int i = 0; i < num_leaves; ++i) {
            const int u = leaves[i];
            for (std::list<int>::const_iterator it = this->children_[u].begin();
                 it != this->children_[u].end(); ++it) {
                --degs[*it];
                if (degs[*it] == 1) { // remember leaves for the next round
                    leaves[num_new_leaves++] =
                        *it; // we can fill the leaves for
                             // the next round into the same
                             // vector from the beginning,
                             // as the number of leaves
                             // decreases in every round
                }
            }
            if (u != this->root_) {
                --degs[this->parent_[u]];
                if (degs[this->parent_[u]] ==
                    1) { // remember leaves for the next round
                    leaves[num_new_leaves++] = this->parent_[u];
                }
            }
        }
        num_vertices_remaining -= num_leaves;
        num_leaves = num_new_leaves;
        num_new_leaves = 0;
    }
    assert((num_leaves >= 1) && (num_leaves <= 2));

    if (num_leaves == 1) {
        c1 = leaves[0];
        c2 = -1;
    } else {
        c1 = leaves[0];
        c2 = leaves[1];
    }
}

bool Tree::is_flip_tree_tau() {
    if (is_star()) {
        return false;
    }

    // remember root and its leftmost child
    const int r = root_;
    const int u = ith_child(root_, 0);

    const int num_bits = 2 * (this->num_vertices_ - 1);
    int this_bitstring[num_bits];
    int canon_bitstring[num_bits];

    int v = ith_child(root_, 0);
    if ((num_children(v) == 1) && (num_children(ith_child(v, 0)) == 0)) {
        // tree has the form 1100...
        // compute my bitstring representation
        to_bitstring(this_bitstring);
        // compute canonically rooted version of myself
        root_canonically();
        // rotate until tree has the form 1100... again
        v = ith_child(root_, 0);
        while ((num_children(v) != 1) || (num_children(ith_child(v, 0)) != 0)) {
            rotate();
            v = ith_child(root_, 0);
        };
    } else {
        if (has_thin_leaf()) { // tree should not have thin leaves
            return false;
        }
        v = ith_child(root_, 0);
        int c = count_pending_edges(v);
        if ((c < num_children(v)) || (c < 2) || (is_light_dumbbell())) {
            return false;
        }
        // tree has the form 1(10)^k0... with k>=2
        // compute my bitstring representation
        to_bitstring(this_bitstring);
        // compute canonically rooted version of myself
        root_canonically();
        // rotate until tree has the form 1(10)^k0... with k>=2 again
        v = ith_child(root_, 0);
        c = count_pending_edges(v);
        while ((c < num_children(v)) || (c < 2)) {
            rotate();
            rotate_children(c);
            v = ith_child(root_, 0);
            c = count_pending_edges(v);
        }
    }

    // compute bitstring representation of canonically rooted version
    to_bitstring(canon_bitstring);

    // restore tree to original state
    rotate_to_vertex(r);
    while (ith_child(root_, 0) != u) {
        rotate_children();
    }

    // compare bitstrings
    if (bitstrings_equal(this_bitstring, canon_bitstring, num_bits)) {
        return true;
    } else {
        return false;
    }
}

bool Tree::is_star() const {
    if ((this->num_vertices_ <= 3) ||
        (deg(this->root_) == this->num_vertices_ - 1) ||
        (deg(ith_child(root_, 0)) == this->num_vertices_ - 1)) {
        return true;
    }
    return false;
}

bool Tree::is_light_dumbbell() const {
    if (this->num_vertices_ < 5) {
        return false;
    }
    const int u = ith_child(root_, 0);
    const int k = num_children(u);
    const int l = num_children(this->root_) - 1;
    if ((k + l + 1 < this->num_vertices_ - 1) || (k <= l)) {
        return false;
    } else {
        return true;
    }
}

bool Tree::is_thin_leaf(int u) const {
    if (deg(u) > 1)
        return false;
    if (((u == this->root_) && (deg(ith_child(u, 0)) == 2)) ||
        ((u != this->root_) && (deg(this->parent_[u]) == 2))) {
        return true;
    } else {
        return false;
    }
}

bool Tree::has_thin_leaf() const {
    for (int i = 0; i < num_vertices_; ++i) {
        if (is_thin_leaf(i)) {
            return true;
        }
    }
    return false;
}

int Tree::count_pending_edges(int u) const {
    int c = 0;
    for (int i = 0; i < num_children(u); ++i) {
        const int v = ith_child(u, i);
        if (num_children(v) == 0) {
            ++c;
        } else {
            return c;
        }
    }
    return c;
}

void Tree::to_bitstring(int x[]) const {
    int pos = 0;
    to_bitstring_rec(x, root_, pos);
}

void Tree::to_bitstring_rec(int x[], int u, int &pos) const {
    if (num_children(u) == 0) {
        return;
    } else {
        for (std::list<int>::const_iterator it = this->children_[u].begin();
             it != this->children_[u].end(); ++it) {
            x[pos++] = 1;
            to_bitstring_rec(x, *it, pos);
            x[pos++] = 0;
        }
    }
}

// The source code for this implementation of Booth's
// algorithm was copied verbatim from the following Wikipedia
// site: "Lexicographically minimal string rotation"
int Tree::min_string_rotation(int x[], int length) {
    // concatenate array with itself to avoid modular arithmetic
    int xx[2 * length];
    std::memcpy(xx, x, sizeof(int) * length);
    std::memcpy(xx + length, x, sizeof(int) * length);
    // failure function
    std::vector<int> fail(2 * length, -1);
    int k = 0; // lexicographically smallest starting position found so far
    for (int j = 1; j < 2 * length; ++j) {
        const int xj = xx[j];
        int i = fail[j - k - 1];
        while ((i != -1) && (xj != xx[k + i + 1])) {
            if (xj < xx[k + i + 1]) {
                k = j - i - 1;
            }
            i = fail[i];
        }
        if (xj != xx[k + i + 1]) {
            if (xj < xx[k]) {
                k = j;
            }
            fail[j - k] = -1;
        } else {
            fail[j - k] = i + 1;
        }
    }
    return k;
}
