// -*- coding: utf-8 -*-
/**
 * @file test_array_like.cpp
 * @brief Unit tests for ShiftArray container adapter
 */
#include <doctest/doctest.h>

#include <ckpttn/array_like.hpp>
#include <vector>

TEST_CASE("ShiftArray: default construction") {
    ShiftArray<std::vector<int>> arr;
    CHECK_EQ(arr.size(), 0);
}

TEST_CASE("ShiftArray: constructed from vector") {
    auto vec = std::vector<int>{10, 20, 30, 40, 50};
    ShiftArray<std::vector<int>> arr(std::move(vec));
    CHECK_EQ(arr.size(), 5);
}

TEST_CASE("ShiftArray: access without shift") {
    auto vec = std::vector<int>{10, 20, 30};
    ShiftArray<std::vector<int>> arr(std::move(vec));
    arr.set_start(0);
    CHECK_EQ(arr[0], 10);
    CHECK_EQ(arr[1], 20);
    CHECK_EQ(arr[2], 30);
}

TEST_CASE("ShiftArray: access with positive shift") {
    auto vec = std::vector<int>{10, 20, 30};
    ShiftArray<std::vector<int>> arr(std::move(vec));
    arr.set_start(5);
    CHECK_EQ(arr[5], 10);
    CHECK_EQ(arr[6], 20);
    CHECK_EQ(arr[7], 30);
}

TEST_CASE("ShiftArray: mutable access") {
    auto vec = std::vector<int>{10, 20, 30};
    ShiftArray<std::vector<int>> arr(std::move(vec));
    arr.set_start(1);
    arr[1] = 99;
    CHECK_EQ(arr[1], 99);
    CHECK_EQ(arr[2], 20);
}

TEST_CASE("ShiftArray: const access") {
    auto vec = std::vector<int>{10, 20, 30};
    const ShiftArray<std::vector<int>> arr(std::move(vec));
    CHECK_EQ(arr[0], 10);
}

TEST_CASE("ShiftArray: resize and set_start") {
    ShiftArray<std::vector<int>> arr;
    arr.set_start(0);
    arr.resize(3, 1);
    CHECK_EQ(arr.size(), 3);
    CHECK_EQ(arr[0], 1);
    CHECK_EQ(arr[1], 1);
    CHECK_EQ(arr[2], 1);
}

TEST_CASE("ShiftArray: resize with shift") {
    ShiftArray<std::vector<int>> arr;
    arr.set_start(10);
    arr.resize(3, 42);
    CHECK_EQ(arr.size(), 3);
    CHECK_EQ(arr[10], 42);
    CHECK_EQ(arr[11], 42);
    CHECK_EQ(arr[12], 42);
}

TEST_CASE("ShiftArray: set_start after data") {
    auto vec = std::vector<int>{100, 200, 300, 400};
    ShiftArray<std::vector<int>> arr(std::move(vec));
    arr.set_start(2);
    CHECK_EQ(arr[2], 100);
    CHECK_EQ(arr[3], 200);
    CHECK_EQ(arr[4], 300);
    CHECK_EQ(arr[5], 400);
}

TEST_CASE("ShiftArray: multiple shift starts") {
    auto vec = std::vector<int>{5, 6, 7};
    ShiftArray<std::vector<int>> arr(std::move(vec));
    arr.set_start(0);
    CHECK_EQ(arr[0], 5);
    arr.set_start(3);
    CHECK_EQ(arr[3], 5);
    CHECK_EQ(arr[4], 6);
    CHECK_EQ(arr[5], 7);
    arr.set_start(1);
    CHECK_EQ(arr[1], 5);
    CHECK_EQ(arr[2], 6);
    CHECK_EQ(arr[3], 7);
}
