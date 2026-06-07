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
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <vector>

// display help
void help() {
    std::cout << "./middle [options]  compute middle levels Gray code from "
                 "[Muetze,Nummenpalo]"
              << std::endl;
    std::cout << "-h                  display this help" << std::endl;
    std::cout
        << "-n{1,2,...}         list bitstrings of length 2n+1 with weight "
           "n or n+1"
        << std::endl;
    std::cout << "-l{-1,0,1,2,...}    number of bitstrings to list; -1 for "
                 "full cycle"
              << std::endl;
    std::cout << "-v{0,1}^{2n+1}      initial bitstring (length 2n+1, "
                 "weight n or n+1)"
              << std::endl;
    std::cout << "-s{0,1}             store and print all visited bitstrings "
                 "(no=0, yes=1)"
              << std::endl;
    std::cout << "-p{0,1}             print the flip positions instead of "
                 "bitstrings (no=0, yes=1)"
              << std::endl;
    std::cout << "examples:  ./middle -n2" << std::endl;
    std::cout << "           ./middle -n2 -v01010" << std::endl;
    std::cout << "           ./middle -n2 -p1" << std::endl;
    std::cout << "           ./middle -n10 -l50" << std::endl;
    std::cout << "           ./middle -n12 -s0" << std::endl;
}

void opt_n_missing() {
    std::cerr << "option -n is mandatory and must come before -v" << std::endl;
}

void opt_v_error() {
    std::cerr
        << "option -v must be followed by a bitstring of length 2n+1 with "
           "weight n or n+1"
        << std::endl;
}

// user-defined visit function
// y is the current vertex and i is the position of the last flip
void visit_f_empty(const std::vector<int> &y, int i) {
    // visit vertex y
}

std::vector<int> flip_seq_; // flip sequence

void visit_f_log(const std::vector<int> &y, int i) { flip_seq_.push_back(i); }

int main(int argc, char *argv[]) {
    int n;
    bool n_set = false;          // flag whether option -n is present
    long long limit = -1;        // compute all vertices by default
    std::vector<int> v;          // starting vertex
    bool v_set = false;          // flag whether option -v is present
    bool store_vertices = true;  // store vertices by default
    bool print_flip_pos = false; // print bitstrings by default

    // process command line options
    int c;
    while ((c = getopt(argc, argv, ":hn:l:v:s:p:")) != -1) {
        switch (c) {
        case 'h':
            help();
            return 0;
        case 'n':
            n = atoi(optarg);
            if (n < 1) {
                std::cerr << "option -n must be followed by an "
                             "integer from {1,2,...}"
                          << std::endl;
                return 1;
            }
            v.resize(2 * n + 1, 0);
            n_set = true;
            break;
        case 'l':
            limit = atoi(optarg);
            if (limit < -1) {
                std::cerr << "option -l must be followed by an "
                             "integer from {-1,0,1,2,...}"
                          << std::endl;
                return 1;
            }
            break;
        case 'v': {
            if (!n_set) {
                opt_n_missing();
                help();
                return 1;
            }
            char *p = optarg;
            int length = 0;
            int num_ones = 0;
            // parse bitstring
            while (*p != 0) {
                if ((*p == '0') || (*p == '1')) {
                    v[length] = ((int)(*p)) - 48; // convert ASCII
                                                  // character to bit
                    length++;
                    num_ones += ((*p) - 48);
                    if (length > 2 * n + 1) {
                        opt_v_error();
                        return 1;
                    }
                } else {
                    opt_v_error();
                    return 1;
                }
                p++;
            }
            // check length and weight of bitstring
            if ((length != 2 * n + 1) || (num_ones < n) || (num_ones > n + 1)) {
                opt_v_error();
                return 1;
            }
            v_set = true;
            break;
        }
        case 's': {
            const int arg = atoi(optarg);
            if ((arg < 0) || (arg > 1)) {
                std::cerr << "option -s must be followed by 0 or 1"
                          << std::endl;
                return 1;
            }
            store_vertices = (bool)arg;
            break;
        }
        case 'p': {
            const int arg = atoi(optarg);
            if ((arg < 0) || (arg > 1)) {
                std::cerr << "option -p must be followed by 0 or 1"
                          << std::endl;
                return 1;
            }
            print_flip_pos = (bool)arg;
            break;
        }
        case ':':
            std::cerr << "option -" << (char)optopt << " requires an operand"
                      << std::endl;
            return 1;
        case '?':
            std::cerr << "unrecognized option -" << (char)optopt << std::endl;
            return 1;
        }
    }
    if (!n_set) {
        opt_n_missing();
        help();
        return 1;
    }

    // define a default starting vertex
    if (!v_set) {
        for (int i = 0; i < n; ++i) {
            v[i] = 1;
        }
    }
    Vertex x(v);

    // Starting from x, compute the next limit vertices on a Hamilton cycle.
    // A value limit < 0 means computing until we are back at the starting
    // vertex.
    visit_f_t visit_f = store_vertices ? visit_f_log : visit_f_empty;
    HamCycle hc(x, limit, visit_f);

    // print vertices encountered along the cycle
    if (store_vertices) {
        if (limit != 0) {
            std::cout << x << std::endl;
        }
        for (int j = 0; j < (int)flip_seq_.size() - 1; ++j) {
            const int i = flip_seq_[j];
            x[i] = 1 - x[i];      // flip bit
            if (print_flip_pos) { // print only flip positions
                std::cout << i << std::endl;
            } else { // print actual bitstring
                std::cout << x << std::endl;
            }
        }
        if (limit == flip_seq_.size()) {
            std::cout << "output limit reached" << std::endl;
        }
    }

    return 0;
}
