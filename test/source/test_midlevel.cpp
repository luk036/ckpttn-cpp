// -*- coding: utf-8 -*-
/**
 * @file test_midlevel.cpp
 * @brief Unit tests for MidVertex and MidHamCycle (middle-level Gray code)
 */
#include <doctest/doctest.h>

#include <algorithm>
#include <ckpttn/midlevel/hamcycle.hpp>
#include <ckpttn/midlevel/vertex.hpp>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
// MidVertex construction and basic access
// ============================================================================

TEST_CASE("MidVertex: construction with valid odd-length vector") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    CHECK_NOTHROW(MidVertex{bits});
}

TEST_CASE("MidVertex: operator[] and size") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex v(bits);
    CHECK_EQ(v.size(), 5);
    CHECK_EQ(v[0], 1);
    CHECK_EQ(v[1], 1);
    CHECK_EQ(v[2], 0);
    CHECK_EQ(v[3], 0);
    CHECK_EQ(v[4], 0);
}

TEST_CASE("MidVertex: mutable operator[]") {
    auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex v(bits);
    v[2] = 1;
    CHECK_EQ(v[2], 1);
}

TEST_CASE("MidVertex: get_bits") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    const MidVertex v(bits);
    CHECK_EQ(v.get_bits(), bits);
}

TEST_CASE("MidVertex: const size") {
    const auto bits = std::vector<int>{1, 1, 1, 0, 0, 0, 0};
    const MidVertex v(bits);
    CHECK_EQ(v.size(), 7);
}

TEST_CASE("MidVertex: const operator[]") {
    const auto bits = std::vector<int>{1, 0, 0, 1, 0};
    const MidVertex v(bits);
    CHECK_EQ(v[3], 1);
}

// ============================================================================
// MidVertex: is_first_vertex, is_last_vertex
// ============================================================================

TEST_CASE("MidVertex: is_first_vertex returns true") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    const MidVertex v(bits);
    CHECK(v.is_first_vertex());
    CHECK_FALSE(v.is_last_vertex());
}

TEST_CASE("MidVertex: is_last_vertex returns true") {
    const auto bits = std::vector<int>{1, 0, 0, 1, 0};
    const MidVertex v(bits);
    CHECK(v.is_last_vertex());
    CHECK_FALSE(v.is_first_vertex());
}

TEST_CASE("MidVertex: neither first nor last") {
    const auto bits = std::vector<int>{0, 1, 0, 1, 0};
    const MidVertex v(bits);
    CHECK_FALSE(v.is_first_vertex());
    CHECK_FALSE(v.is_last_vertex());
}

// ============================================================================
// MidVertex: to_first_vertex, to_last_vertex
// ============================================================================

TEST_CASE("MidVertex: to_first_vertex from last vertex") {
    auto bits = std::vector<int>{1, 0, 0, 1, 0};
    MidVertex v(bits);
    const auto steps = v.to_first_vertex();
    CHECK(v.is_first_vertex());
    CHECK_GT(steps, 0);
}

TEST_CASE("MidVertex: to_first_vertex from non-first vertex") {
    auto bits = std::vector<int>{0, 1, 1, 0, 1, 0, 0};
    MidVertex v(bits);
    const auto steps = v.to_first_vertex();
    CHECK(v.is_first_vertex());
    CHECK_GT(steps, 0);
}

TEST_CASE("MidVertex: to_last_vertex from first vertex") {
    auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex v(bits);
    const auto steps = v.to_last_vertex();
    CHECK(v.is_last_vertex());
    CHECK_GT(steps, 0);
}

TEST_CASE("MidVertex: to_last_vertex round-trip") {
    auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex v(bits);
    v.to_last_vertex();
    v.to_first_vertex();
    CHECK(v.is_first_vertex());
}

// ============================================================================
// MidVertex: operator<< and comparisons
// ============================================================================

TEST_CASE("MidVertex: operator<<") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    const MidVertex v(bits);
    std::ostringstream oss;
    oss << v;
    CHECK_EQ(oss.str(), "11000");
}

TEST_CASE("MidVertex: equality operators") {
    const auto bits1 = std::vector<int>{1, 1, 0, 0, 0};
    const auto bits2 = std::vector<int>{1, 1, 0, 0, 0};
    const auto bits3 = std::vector<int>{1, 0, 1, 0, 0};
    const MidVertex v1(bits1);
    const MidVertex v2(bits2);
    const MidVertex v3(bits3);
    CHECK(v1 == v2);
    CHECK(v1 != v3);
}

// ============================================================================
// Free functions: bitstrings_less_than, bitstrings_equal
// ============================================================================

TEST_CASE("bitstrings_less_than") {
    auto a = std::vector<int>{1, 1, 0, 0, 0};
    auto b = std::vector<int>{1, 0, 1, 0, 0};
    CHECK(bitstrings_less_than(b.data(), a.data(), 5));
    CHECK_FALSE(bitstrings_less_than(a.data(), b.data(), 5));
}

TEST_CASE("bitstrings_less_than equal") {
    auto a = std::vector<int>{1, 1, 0, 0, 0};
    CHECK_FALSE(bitstrings_less_than(a.data(), a.data(), 5));
}

TEST_CASE("bitstrings_equal") {
    auto a = std::vector<int>{1, 1, 0, 0, 0};
    auto b = std::vector<int>{1, 0, 1, 0, 0};
    CHECK(bitstrings_equal(a.data(), a.data(), 5));
    CHECK_FALSE(bitstrings_equal(a.data(), b.data(), 5));
}

// ============================================================================
// MidVertex: compute_flip_seq_0
// ============================================================================

TEST_CASE("MidVertex: compute_flip_seq_0 without flip") {
    auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex v(bits);
    CHECK(v.is_first_vertex());

    auto seq = std::vector<int>{};
    v.compute_flip_seq_0(seq, false);

    CHECK_EQ(seq.size(), 6);
    CHECK_EQ(seq[0], 3);
    CHECK_EQ(seq[1], 0);
}

TEST_CASE("MidVertex: compute_flip_seq_0 with flip and bits[1]=1") {
    auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex v(bits);
    CHECK(v.is_first_vertex());

    auto seq = std::vector<int>{};
    v.compute_flip_seq_0(seq, true);

    CHECK_EQ(seq.size(), 2);
    CHECK_EQ(seq[0], 2);
    CHECK_EQ(seq[1], 0);
}

TEST_CASE("MidVertex: compute_flip_seq_0 with flip and bits[1]=0") {
    auto bits3 = std::vector<int>{1, 0, 1, 1, 0, 0, 0};
    MidVertex v3(bits3);
    CHECK(v3.is_first_vertex());

    auto seq3 = std::vector<int>{};
    v3.compute_flip_seq_0(seq3, true);

    // sequence should be non-empty with valid indices
    CHECK_GT(seq3.size(), 0);
    CHECK_EQ(seq3[1], 0);  // second element is always 0
    for (auto idx : seq3) {
        CHECK_GE(idx, 0);
        CHECK_LT(idx, v3.size());
    }
}

// ============================================================================
// MidVertex: compute_flip_seq_1
// ============================================================================

TEST_CASE("MidVertex: compute_flip_seq_1") {
    auto bits = std::vector<int>{1, 0, 0, 1, 0};
    MidVertex v(bits);
    CHECK(v.is_last_vertex());

    auto seq = std::vector<int>{};
    v.compute_flip_seq_1(seq);

    CHECK_GT(seq.size(), 0);
    CHECK_EQ(seq[0], 3);
}

// ============================================================================
// MidHamCycle: construction and basic behavior
// ============================================================================

TEST_CASE("MidHamCycle: construction with limit=-1 (full cycle)") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex start(bits);

    auto flip_count = 0;
    auto visit_fn = [&](const std::vector<int>&, int) { ++flip_count; };

    MidHamCycle cycle(start, -1, visit_fn);

    CHECK_GT(cycle.get_length(), 0);
    CHECK_GT(flip_count, 0);
}

TEST_CASE("MidHamCycle: construction with limit=0") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex start(bits);

    auto flip_count = 0;
    auto visit_fn = [&](const std::vector<int>&, int) { ++flip_count; };

    MidHamCycle cycle(start, 0, visit_fn);

    CHECK_EQ(cycle.get_length(), 0);
}

TEST_CASE("MidHamCycle: construction with small positive limit") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex start(bits);

    auto flip_count = 0;
    auto visit_fn = [&](const std::vector<int>&, int) { ++flip_count; };

    MidHamCycle cycle(start, 5, visit_fn);

    CHECK_LE(cycle.get_length(), 5);
}

TEST_CASE("MidHamCycle: bigger instance (n=3)") {
    const auto bits = std::vector<int>{1, 1, 1, 0, 0, 0, 0};
    MidVertex start(bits);

    auto flip_count = 0;
    auto visit_fn = [&](const std::vector<int>&, int) { ++flip_count; };

    MidHamCycle cycle(start, -1, visit_fn);

    CHECK_GT(cycle.get_length(), 0);
    CHECK_GT(flip_count, 0);
}

TEST_CASE("MidHamCycle: visit function receives valid indices") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex start(bits);

    auto max_idx = -1;
    auto visit_fn = [&](const std::vector<int>&, int i) { max_idx = std::max(i, max_idx); };

    MidHamCycle cycle(start, 20, visit_fn);

    CHECK_GE(max_idx, 0);
    CHECK_LE(max_idx, 4);
}

TEST_CASE("MidHamCycle: visit function sees bitstring state") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex start(bits);

    auto visited_strings = std::vector<std::vector<int>>{};
    auto visit_fn = [&](const std::vector<int>& y, int) { visited_strings.push_back(y); };

    MidHamCycle cycle(start, 10, visit_fn);

    CHECK_GT(visited_strings.size(), 0);
    for (const auto& s : visited_strings) {
        CHECK_EQ(s.size(), bits.size());
    }
}

TEST_CASE("MidHamCycle: starting from last vertex") {
    const auto bits = std::vector<int>{1, 0, 0, 1, 0};
    MidVertex start(bits);

    auto flip_count = 0;
    auto visit_fn = [&](const std::vector<int>&, int) { ++flip_count; };

    MidHamCycle cycle(start, 20, visit_fn);

    CHECK_GT(cycle.get_length(), 0);
}
TEST_CASE("MidVertex: operator[] and size") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex v(bits);
    CHECK_EQ(v.size(), 5);
    CHECK_EQ(v[0], 1);
    CHECK_EQ(v[1], 1);
    CHECK_EQ(v[2], 0);
    CHECK_EQ(v[3], 0);
    CHECK_EQ(v[4], 0);
}

TEST_CASE("MidVertex: mutable operator[]") {
    auto bits = std::vector<int>{1, 1, 0, 0, 0};
    MidVertex v(bits);
    v[2] = 1;
    CHECK_EQ(v[2], 1);
}

TEST_CASE("MidVertex: get_bits") {
    const auto bits = std::vector<int>{1, 1, 0, 0, 0};
    const MidVertex v(bits);
    CHECK_EQ(v.get_bits(), bits);
}
