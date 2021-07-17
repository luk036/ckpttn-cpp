#include <array>
#include <ckpttn/set_partition.hpp> // import  setpart
#include <doctest/doctest.h>
// #include <iostream>

TEST_CASE("Test set partition (odd k)")
{
    constexpr auto n = 10;
    constexpr auto k = 5;
    auto cnt = 1;

    std::array<int, n + 1> b {0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4};

    for (const auto& [x, y] : set_partition(n, k))
    {
        // auto old = b[x];
        b[x] = y;
        cnt += 1;
        // std::cout << "Move " << x << " from block " << old << " to " << y
        //           << "\n";
    }

    auto totalNumber = decltype(Stirling2nd<n, k>())::value;
    CHECK(totalNumber == cnt);
}

TEST_CASE("Test set partition (even k)")
{
    constexpr auto n = 9;
    constexpr auto k = 4;
    auto cnt = 1;

    std::array<int, n + 1> b {0, 0, 0, 0, 0, 0, 0, 1, 2};

    for (const auto& [x, y] : set_partition(n, k))
    {
        // auto old = b[x];
        b[x] = y;
        cnt += 1;
        // std::cout << "Move " << x << " from block " << old << " to " << y
        //           << "\n";
    }

    auto totalNumber = decltype(Stirling2nd<n, k>())::value;
    CHECK(totalNumber == cnt);
}