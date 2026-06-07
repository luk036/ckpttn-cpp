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

#ifndef HAMCYCLE_HPP
#define HAMCYCLE_HPP

#include "vertex.hpp"
#include <vector>

// type of user-defined visit function
typedef void (*visit_f_t)(const std::vector<int> &y, int i);

class HamCycle {
  public:
    // Starting from a vertex x of the middle levels graph G_n (the
    // parameter n is implicit in the number 2n+1 of bits of x), compute the
    // next limit vertices on a Hamilton cycle (if the vertex x is reached
    // again, the computation is terminated prematurely). A value limit < 0
    // means computing until we are back at the starting vertex. If
    // store_vertices == true, then the vector of flipped bit positions is
    // stored along the way (otherwise not).
    explicit HamCycle(const Vertex &x, long long limit, visit_f_t visit_f);
    long long get_length() const { return length_; }

  private:
    Vertex x_;          // starting vertex
    Vertex y_;          // current vertex
    long long limit_;   // the number of vertices to be visited
    visit_f_t visit_f_; // user-defined visit function
    long long length_;  // number of vertices visited so far

    // #### auxiliary functions ####

    // Execute the given flip sequence seq on the current vertex x, and do
    // the necessary termination checks along the way. Return value is true
    // if termination criterion is satisfied (break surrounding loop).
    bool flip_seq(const std::vector<int> &seq, int &dist_to_start,
                  bool final_path);
};

#endif
