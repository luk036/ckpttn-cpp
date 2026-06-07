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

#include "hamcycle.hpp"
#include "tree.hpp"
#include "vertex.hpp"
#include <cassert>
#include <vector>

HamCycle::HamCycle(const Vertex &x, long long limit, visit_f_t visit_f)
    : x_(x), y_(x), limit_(limit), visit_f_(visit_f) {
    assert(this->x_.size() % 2 == 1);
    const int n = this->x_.size() / 2;

    // ############################
    // ### Initialization phase ###
    // ############################

    // The vertex xs is the first vertex of a path in H_n\circ 0 for which
    // either this path or the subsequent one in \overline{\rev}(H_n)\circ 1
    // contains the vertex this->x_.

    Vertex xs(this->x_);
    int skip = 0;         // number of vertices which are not visited (skipped)
                          // before reaching the first vertex x_
    if (xs[2 * n] == 1) { // last bit == 1
        // move backwards along the cycle to the first vertex of the
        // path in the graph \overline{\rev}(H_n)\circ 1 (this is the
        // last vertex of this oriented path, as the path is traversed
        // opposite to its orientation)
        xs.rev_inv();
        skip += xs.to_last_vertex();
        xs.rev_inv();
        xs[2 * n] = 0; // jump backwards to the graph H_n\circ 0 by
                       // flipping last bit
        skip++;
    }
    // move backwards along the cycle to the first vertex
    // of the path in the graph H_n\circ 0
    skip += xs.to_first_vertex();
    assert(xs.is_first_vertex());

    // maintain adjacency list representation of the tree
    // that corresponds to the current vertex this->y_ throughout the
    // algorithm
    this->y_ = xs;
    Tree y_tree(this->y_);

    // if initial cycle segment contains flipped paths,
    // we may need to start from the other path
    if ((skip > 0) && (y_tree.flip_tree())) {
        if ((xs[1] == 1) && (skip <= 5)) {
            skip = 6 - skip; // need to correct for the reverse traversal
                             // of the initial part of flipped path
        }
        int y_string[2 * n];
        y_tree.to_bitstring(y_string);
        std::vector<int> y_vec(y_string, y_string + 2 * n);
        y_vec.push_back(0); // add 0-bit at the end
        xs = Vertex(y_vec);
        this->y_ = xs;
    }

    this->length_ = 0;

    // ##################################
    // ### Hamilton cycle computation ###
    // ##################################
    std::vector<int> seq;
    std::vector<int> seq01;
    seq01.push_back(2 * n); // flip sequence that flips only the last bit
    int dist_to_start = skip;
    bool final_path =
        false; // back in the path that contains the starting vertex
    while (true) {
        // #################################################
        // follow the path in the graph H_n\circ 0
        // #################################################

        bool flip = y_tree.flip_tree(); // tau() or tau_inverse() is applied
                                        // inside the function call
        y_tree.rotate();

        // compute flip sequence
        this->y_.compute_flip_seq_0(seq, flip);

        // apply flip sequence
        assert(this->y_.is_first_vertex());
        if (flip_seq(seq, dist_to_start, final_path)) {
            break;
        }
        assert(this->y_.is_last_vertex());

        // flip last bit to jump to the graph \overline{\rev}(H_n)\circ
        // 1
        if (flip_seq(seq01, dist_to_start, final_path)) {
            break;
        }
        assert(this->y_[2 * n] == 1);

        // #################################################
        // follow the path in the graph \overline{\rev}(H_n)\circ 1
        // #################################################

        // ############# ORIGINAL SEQUENCE OF CODE AS DESCRIBED IN THE
        // PAPER
        // ############# compute flip sequence
        /*
        Vertex yp = y_;
        yp.rev_inv();
        yp.to_first_vertex();
        yp.compute_flip_seq_0(seq, false);
        // compute transformed flip sequence under the operations
        // of reversal and complementation (rev_inv)
        std::reverse(seq.begin(), seq.end());
        for (int j = 0; j < seq.size(); ++j) {
          seq[j] = 2*n-1 - seq[j];
        }
        */
        // ############# SPEED-OPTIMIZED VARIANT #############
        // compute transformed flip sequence directly
        this->y_.compute_flip_seq_1(seq);

        // apply flip sequence
        assert(this->y_.is_last_vertex());
        if (flip_seq(seq, dist_to_start, final_path)) {
            break;
        }
        assert(this->y_.is_first_vertex());

        // flip last bit to jump to the graph H_n\circ 0
        if (flip_seq(seq01, dist_to_start, final_path)) {
            break;
        }
        assert(this->y_[2 * n] == 0);

        // exit loop prematurely if starting vertex has been reached
        // again
        if (this->y_ == xs) {
            final_path = true;
            dist_to_start = skip;
        }
    }
}

bool HamCycle::flip_seq(const std::vector<int> &seq, int &dist_to_start,
                        bool final_path) {
    if ((dist_to_start > 0) || final_path ||
        ((this->limit_ >= 0) && (this->length_ + seq.size() >= this->limit_))) {
        // apply only part of the flip sequence
        for (int j = 0; j < seq.size(); ++j) {
            if ((final_path && (dist_to_start == 0)) ||
                ((this->limit_ >= 0) && (this->length_ == this->limit_))) {
                return true; // terminate Hamilton cycle
                             // computation prematurely
            }
            const int i = seq[j];
            if ((dist_to_start == 0) || final_path) {
                this->y_[i] = 1 - this->y_[i];
#ifndef NVISIT
                // the visit_f_() function is useful only when
                // nonempty
                visit_f_(this->y_.get_bits(), i);
#endif
                ++length_;
            } else {
                this->y_[i] = 1 - this->y_[i];
            }
            if (dist_to_start > 0) {
                dist_to_start--;
            }
        }
    } else {
        // highspeed loop without case distinctions
        // apply the entire flip sequence
        for (int j = 0; j < seq.size(); ++j) {
            const int i = seq[j];
            this->y_[i] = 1 - this->y_[i];
#ifndef NVISIT
            // the visit_f_() function is useful only when nonempty
            visit_f_(this->y_.get_bits(), i);
#endif
        }
        this->length_ += seq.size();
    }
    return false; // continue Hamilton cycle computation
}
