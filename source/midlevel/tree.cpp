/*
 * Adapted from: Torsten Muetze, Jerri Nummenpalo (2018)
 * Original license: GPL v2+
 */

#include <algorithm>
#include <cassert>
#include <ckpttn/midlevel/tree.hpp>
#include <ckpttn/midlevel/vertex.hpp>
#include <cstring>
#include <list>
#include <vector>

MidTree::MidTree(const MidVertex& x) {
    const std::vector<int>& xv = x.get_bits();
    assert(xv.size() % 2 == 1);

    this->root_ = 0;
    this->num_vertices_ = static_cast<int>(xv.size() - 1) / 2 + 1;
    this->children_.resize(this->num_vertices_);
    this->parent_.resize(num_vertices_, 0);

    int u = this->root_;
    int n = 1;
    [[maybe_unused]] int height = 0;
    for (int i = 0; i < static_cast<int>(xv.size()) - 1; ++i) {
        if (x[i] == 1) {
            this->children_[u].push_back(n);
            this->parent_[n] = u;
            u = n;
            ++n;
        } else {
            u = this->parent_[u];
        }
        height += (2 * x[i] - 1);
        assert(height >= 0);
    }
    assert(n == this->num_vertices_);
}

int MidTree::deg(int u) const {
    assert((0 <= u) && (u < this->num_vertices_));
    if (u == this->root_) {
        return static_cast<int>(this->children_[u].size());
    }
    return static_cast<int>(this->children_[u].size() + 1);
}

int MidTree::num_children(int u) const {
    assert((0 <= u) && (u < this->num_vertices_));
    return static_cast<int>(this->children_[u].size());
}

int MidTree::ith_child(int u, int i) const {
    assert((0 <= u) && (u < this->num_vertices_));
    assert((0 <= i) && (i < num_children(u)));
    auto it = this->children_[u].begin();
    std::advance(it, i);
    return *it;
}

bool MidTree::is_tau_preimage() const {
    if (this->num_vertices_ < 3) {
        return false;
    }
    const int u = ith_child(root_, 0);
    if (num_children(u) == 0) {
        return false;
    }
    const int v = ith_child(u, 0);
    return num_children(v) == 0;
}

bool MidTree::is_tau_image() const {
    return !((this->num_vertices_ < 3) || (num_children(this->root_) < 2)
             || (num_children(ith_child(root_, 0)) > 0));
}

void MidTree::tau() {
    assert(is_tau_preimage());
    const int u = ith_child(root_, 0);
    const int v = ith_child(u, 0);
    move_leaf(v, root_, 0);
}

void MidTree::tau_inverse() {
    assert(is_tau_image());
    const int v = ith_child(root_, 0);
    const int u = ith_child(root_, 1);
    move_leaf(v, u, 0);
}

void MidTree::move_leaf(int leaf, int new_parent, int pos) {
    assert((0 <= leaf) && (leaf < this->num_vertices_));
    assert((0 <= new_parent) && (new_parent < this->num_vertices_));
    assert((0 <= pos) && (pos <= static_cast<int>(this->children_[new_parent].size())));
    assert(num_children(leaf) == 0);
    const int old_parent = this->parent_[leaf];
    for (auto it = this->children_[old_parent].begin(); it != this->children_[old_parent].end();
         ++it) {
        if (*it == leaf) {
            this->children_[old_parent].erase(it);
            break;
        }
    }
    auto it = this->children_[new_parent].begin();
    std::advance(it, pos);
    this->children_[new_parent].insert(it, leaf);
    this->parent_[leaf] = new_parent;
}

void MidTree::rotate() {
    assert(this->num_vertices_ >= 2);
    const int u = ith_child(root_, 0);
    this->parent_[root_] = u;
    this->children_[u].splice(this->children_[u].end(), this->children_[root_],
                              this->children_[root_].begin());
    this->children_[u].back() = root_;
    this->root_ = u;
}

void MidTree::rotate_to_vertex(int u) {
    while (this->root_ != u) {
        rotate();
    }
}

void MidTree::rotate_children() { rotate_children(1); }

void MidTree::rotate_children(int k) {
    auto it = this->children_[root_].begin();
    std::advance(it, k);
    std::rotate(this->children_[root_].begin(), it, this->children_[root_].end());
}

bool MidTree::flip_tree() {
    if (is_tau_preimage() && is_flip_tree_tau()) {
        tau();
        return true;
    }
    if (is_tau_image()) {
        tau_inverse();
        if (is_flip_tree_tau()) {
            return true;
        }
        tau();
    }
    return false;
}

void MidTree::root_canonically() {
    int c1;
    int c2;
    compute_center(c1, c2);
    if (c2 != -1) {
        const int num_bits = 2 * (this->num_vertices_ - 1);
        std::vector<int> x1(num_bits);
        std::vector<int> x2(num_bits);
        rotate_to_vertex(c1);
        while (ith_child(root_, 0) != c2) {
            rotate_children();
        }
        to_bitstring(x1.data());

        rotate();
        rotate_children(num_children(this->root_) - 1);
        assert((this->root_ == c2) && (ith_child(root_, 0) == c1));
        to_bitstring(x2.data());

        if (bitstrings_less_than(x1.data(), x2.data(), num_bits)) {
            rotate();
            rotate_children(num_children(this->root_) - 1);
            assert((this->root_ == c1) && (ith_child(root_, 0) == c2));
        }
    } else {
        rotate_to_vertex(c1);
        const int num_bits = 2 * (this->num_vertices_ - 1);
        std::vector<int> x(num_bits);
        to_bitstring(x.data());

        std::vector<int> subtree_count(num_bits);
        int c = 0;
        int depth = 0;
        for (int i = 0; i < num_bits; ++i) {
            if (x[i] == 1) {
                ++depth;
            } else {
                --depth;
            }
            subtree_count[i] = c;
            if (depth == 0) {
                ++c;
            }
        }
        assert(c == num_children(this->root_));

        const int k = min_string_rotation(x.data(), num_bits);
        rotate_children(subtree_count[k]);
    }
}

void MidTree::compute_center(int& c1, int& c2) const {
    std::vector<int> degs(num_vertices_, 0);
    std::vector<int> leaves(num_vertices_, 0);
    int num_leaves = 0;
    for (int i = 0; i < num_vertices_; ++i) {
        degs[i] = deg(i);
        if (degs[i] == 1) {
            leaves[num_leaves++] = i;
        }
    }

    int num_vertices_remaining = num_vertices_;
    int num_new_leaves = 0;
    while (num_vertices_remaining > 2) {
        for (int i = 0; i < num_leaves; ++i) {
            const int u = leaves[i];
            for (int it : this->children_[u]) {
                --degs[it];
                if (degs[it] == 1) {
                    leaves[num_new_leaves++] = it;
                }
            }
            if (u != this->root_) {
                --degs[this->parent_[u]];
                if (degs[this->parent_[u]] == 1) {
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

bool MidTree::is_flip_tree_tau() {
    if (is_star()) {
        return false;
    }

    const int r = root_;
    const int u = ith_child(root_, 0);

    const int num_bits = 2 * (this->num_vertices_ - 1);
    std::vector<int> this_bitstring(num_bits);
    std::vector<int> canon_bitstring(num_bits);

    int v = ith_child(root_, 0);
    if ((num_children(v) == 1) && (num_children(ith_child(v, 0)) == 0)) {
        to_bitstring(this_bitstring.data());
        root_canonically();
        v = ith_child(root_, 0);
        while ((num_children(v) != 1) || (num_children(ith_child(v, 0)) != 0)) {
            rotate();
            v = ith_child(root_, 0);
        };
    } else {
        if (has_thin_leaf()) {
            return false;
        }
        v = ith_child(root_, 0);
        int c = count_pending_edges(v);
        if ((c < num_children(v)) || (c < 2) || (is_light_dumbbell())) {
            return false;
        }
        to_bitstring(this_bitstring.data());
        root_canonically();
        v = ith_child(root_, 0);
        c = count_pending_edges(v);
        while ((c < num_children(v)) || (c < 2)) {
            rotate();
            rotate_children(c);
            v = ith_child(root_, 0);
            c = count_pending_edges(v);
        }
    }

    to_bitstring(canon_bitstring.data());

    rotate_to_vertex(r);
    while (ith_child(root_, 0) != u) {
        rotate_children();
    }

    return bitstrings_equal(this_bitstring.data(), canon_bitstring.data(), num_bits);
}

bool MidTree::is_star() const {
    return (this->num_vertices_ <= 3) || (deg(this->root_) == this->num_vertices_ - 1)
           || (deg(ith_child(root_, 0)) == this->num_vertices_ - 1);
}

bool MidTree::is_light_dumbbell() const {
    if (this->num_vertices_ < 5) {
        return false;
    }
    const int u = ith_child(root_, 0);
    const int k = num_children(u);
    const int l = num_children(this->root_) - 1;
    return !((k + l + 1 < this->num_vertices_ - 1) || (k <= l));
}

bool MidTree::is_thin_leaf(int u) const {
    if (deg(u) > 1) return false;
    return ((u == this->root_) && (deg(ith_child(u, 0)) == 2))
           || ((u != this->root_) && (deg(this->parent_[u]) == 2));
}

bool MidTree::has_thin_leaf() const {
    for (int i = 0; i < num_vertices_; ++i) {
        if (is_thin_leaf(i)) {
            return true;
        }
    }
    return false;
}

int MidTree::count_pending_edges(int u) const {
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

void MidTree::to_bitstring(int x[]) const {
    int pos = 0;
    to_bitstring_rec(x, root_, pos);
}

void MidTree::to_bitstring_rec(int x[], int u, int& pos) const {
    if (num_children(u) == 0) {
        return;
    }
    for (std::list<int>::const_iterator it = this->children_[u].begin();
         it != this->children_[u].end(); ++it) {
        x[pos++] = 1;
        to_bitstring_rec(x, *it, pos);
        x[pos++] = 0;
    }
}

int MidTree::min_string_rotation(int x[], int length) {
    std::vector<int> xx(2 * length);
    std::memcpy(xx.data(), x, sizeof(int) * length);
    std::memcpy(xx.data() + length, x, sizeof(int) * length);
    std::vector<int> fail(2 * length, -1);
    int k = 0;
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
