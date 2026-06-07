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

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// Include the original code with private members exposed
#define private public
#define protected public
#include "hamcycle.hpp"
#include "tree.hpp"
#include "vertex.hpp"
#undef private
#undef protected

// Test utilities

/**
 * @brief Print a test header.
 * @param test_name Name of the test
 */
void print_test_header(const std::string &test_name) {
  std::cout << "=== TEST: " << test_name << " ===\n";
}

/**
 * @brief Print a test result.
 * @param passed Whether the test passed
 * @param message Optional message to display
 */
void print_test_result(bool passed, const std::string &message = "") {
  if (passed) {
    std::cout << "[PASS]";
  } else {
    std::cout << "[FAIL]";
  }
  if (!message.empty()) {
    std::cout << " - " << message;
  }
  std::cout << "\n";
}

/**
 * @brief Test vertex construction from bitstring.
 * @return true if test passed
 */
bool test_vertex_construction() {
  print_test_header("Vertex Construction");
  bool passed = true;

  std::vector<int> bits = {1, 0, 1, 0, 1};
  Vertex v(bits);

  if (v.bits_ != bits) {
    print_test_result(false, "Bits not initialized correctly");
    passed = false;
  }

  print_test_result(passed);
  return passed;
}

/**
 * @brief Test vertex operations (size, indexing, rev_inv).
 * @return true if test passed
 */
bool test_vertex_operations() {
  print_test_header("Vertex Operations");
  bool passed = true;

  std::vector<int> bits = {1, 0, 1, 0, 1};
  Vertex v(bits);

  // Test size
  if (v.size() != 5) {
    print_test_result(false, "Incorrect size");
    passed = false;
  }

  // Test indexing
  if (v[0] != 1 || v[1] != 0) {
    print_test_result(false, "Incorrect indexing");
    passed = false;
  }

  v[1] = 1;
  if (v[1] != 1) {
    print_test_result(false, "Incorrect assignment");
    passed = false;
  }

  // Test reverse and invert
  v.rev_inv();
  std::vector<int> expected = {1, 0, 0, 0, 1};
  if (v.bits_ != expected) {
    print_test_result(false, "Incorrect rev_inv");
    passed = false;
  }

  // Test partial reverse and invert
  v.rev_inv(1, 3);
  expected = {1, 1, 1, 1, 1};
  if (v.bits_ != expected) {
    print_test_result(false, "Incorrect partial rev_inv");
    passed = false;
  }

  print_test_result(passed);
  return passed;
}

/**
 * @brief Test vertex path operations (first/last vertex, touchdown, dive).
 * @return true if test passed
 */
bool test_vertex_path_operations() {
  print_test_header("Vertex Path Operations");
  bool passed = true;

  // Test first vertex identification
  Vertex first({1, 1, 0, 0, 1});
  if (!first.is_first_vertex() || first.is_last_vertex()) {
    print_test_result(false, "First vertex identification failed");
    passed = false;
  }

  // Test last vertex identification
  Vertex last({1, 0, 1, 0, 1});
  if (!last.is_last_vertex() || last.is_first_vertex()) {
    print_test_result(false, "Last vertex identification failed");
    passed = false;
  }

  // Test first touchdown
  Vertex v1({1, 1, 0, 0, 1});
  if (v1.first_touchdown(0) != 3) {
    print_test_result(false, "First touchdown failed");
    passed = false;
  }

  // Test first dive
  Vertex v2({0, 0, 1, 1, 1});
  if (v2.first_dive() != 1) {
    print_test_result(false, "First dive failed");
    passed = false;
  }

  print_test_result(passed);
  return passed;
}

/**
 * @brief Test flip sequence generation.
 * @return true if test passed
 */
bool test_flip_sequence_generation() {
  print_test_header("Flip Sequence Generation");
  bool passed = true;

  Vertex first({1, 1, 0, 0, 1});
  std::vector<int> seq;

  // Test compute_flip_seq_0
  first.compute_flip_seq_0(seq, false);
  if (seq.size() != 4) {
    print_test_result(false, "Incorrect flip seq 0 size");
    passed = false;
  }

  // Test compute_flip_seq_1
  Vertex last({1, 0, 1, 0, 1});
  seq.clear();
  last.compute_flip_seq_1(seq);
  if (seq.size() != 4) {
    print_test_result(false, "Incorrect flip seq 1 size");
    passed = false;
  }

  print_test_result(passed);
  return passed;
}

/**
 * @brief Test tree construction from vertex.
 * @return true if test passed
 */
bool test_tree_construction() {
  print_test_header("Tree Construction");
  bool passed = true;

  Vertex v({1, 1, 0, 0, 1});
  Tree t(v);

  if (t.num_vertices_ != 3 || t.root_ != 0 || t.children_[0].size() != 2) {
    print_test_result(false, "Tree construction failed");
    passed = false;
  }

  print_test_result(passed);
  return passed;
}

/**
 * @brief Test tree operations (degree, child access, tau).
 * @return true if test passed
 */
bool test_tree_operations() {
  print_test_header("Tree Operations");
  bool passed = true;

  Vertex v({1, 1, 0, 0, 1});
  Tree t(v);

  // Test degree
  if (t.deg(0) != 2 || t.deg(1) != 1) {
    print_test_result(false, "Degree calculation failed");
    passed = false;
  }

  // Test child access
  if (t.ith_child(0, 0) != 1 || t.ith_child(0, 1) != 2) {
    print_test_result(false, "Child access failed");
    passed = false;
  }

  // Test tau operations
  Vertex tau_v({1, 1, 1, 0, 0, 0, 1});
  Tree tau_tree(tau_v);
  if (!tau_tree.is_tau_preimage() || tau_tree.is_tau_image()) {
    print_test_result(false, "Tau operations failed");
    passed = false;
  }

  print_test_result(passed);
  return passed;
}

/**
 * @brief Test tree transformations (rotation, leaf movement).
 * @return true if test passed
 */
bool test_tree_transformations() {
  print_test_header("Tree Transformations");
  bool passed = true;

  Vertex v({1, 1, 1, 0, 0, 0, 1});
  Tree t(v);

  // Test rotation
  t.rotate();
  if (t.root_ != 1 || t.parent_[0] != 1) {
    print_test_result(false, "Rotation failed");
    passed = false;
  }

  // Test leaf movement
  t.move_leaf(3, 0, 1);
  if (t.parent_[3] != 0 || t.children_[0].size() != 3) {
    print_test_result(false, "Leaf movement failed");
    passed = false;
  }

  print_test_result(passed);
  return passed;
}

/**
 * @brief Test Hamilton cycle construction.
 * @return true if test passed
 */
bool test_hamilton_cycle() {
  print_test_header("Hamilton Cycle");
  bool passed = true;

  Vertex v({1, 1, 0, 0, 1});
  auto visit_func = [](const std::vector<int> &, int) {};
  HamCycle hc(v, -1, visit_func);

  if (hc.x_.bits_ != v.get_bits() || hc.y_.bits_ != v.get_bits()) {
    print_test_result(false, "Cycle construction failed");
    passed = false;
  }

  print_test_result(passed);
  return passed;
}

/**
 * @brief Test bitstring comparison functions.
 * @return true if test passed
 */
bool test_bitstring_comparison() {
  print_test_header("Bitstring Comparison");
  bool passed = true;

  int x[] = {1, 0, 1};
  int y[] = {1, 1, 0};

  if (!bitstrings_less_than(x, y, 3) || bitstrings_less_than(y, x, 3) ||
      !bitstrings_equal(x, x, 3) || bitstrings_equal(x, y, 3)) {
    print_test_result(false, "Bitstring comparison failed");
    passed = false;
  }

  print_test_result(passed);
  return passed;
}

/**
 * @brief Main entry point for the test suite.
 * @return 0 if all tests passed, 1 otherwise
 */
int main() {
  int passed = 0;
  int total = 0;

  auto run_test = [&](bool (*test_func)()) {
    total++;
    if (test_func()) {
      passed++;
    }
  };

  run_test(test_vertex_construction);
  run_test(test_vertex_operations);
  run_test(test_vertex_path_operations);
  run_test(test_flip_sequence_generation);
  run_test(test_tree_construction);
  run_test(test_tree_operations);
  run_test(test_tree_transformations);
  run_test(test_hamilton_cycle);
  run_test(test_bitstring_comparison);

  std::cout << "\n=== TEST SUMMARY ===\n";
  std::cout << "Passed: " << passed << "/" << total << "\n";
  std::cout << "Success rate: " << (passed * 100 / total) << "%\n";

  return passed == total ? 0 : 1;
}
