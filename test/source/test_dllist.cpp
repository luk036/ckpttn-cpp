#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expr...
// #include <__config>                        // for std
#include <cinttypes>          // for uint8_t
#include <ckpttn/dllist.hpp>  // for dllink, dll_iterator, oper...
#include <ckpttn/robin.hpp>   // for robin, robin<>::iterable_w...
#include <utility>            // for pair

using namespace std;

TEST_CASE("Test dllist") {
    auto L1 = dllink<std::pair<int, int>>{std::make_pair(0, 0)};
    auto L2 = dllink<std::pair<int, int>>{std::make_pair(0, 0)};
    auto d = dllink<std::pair<int, int>>{std::make_pair(0, 0)};
    auto e = dllink<std::pair<int, int>>{std::make_pair(0, 0)};
    auto f = dllink<std::pair<int, int>>{std::make_pair(0, 0)};
    CHECK(L1.is_empty());

    L1.appendleft(e);
    CHECK(!L1.is_empty());

    L1.appendleft(f);
    L1.append(d);
    L2.append(L1.pop());
    L2.append(L1.popleft());
    CHECK(!L1.is_empty());
    CHECK(!L2.is_empty());

    auto count = 0U;
    for (auto& _d : L2) {
        static_assert(sizeof _d >= 0, "make compiler happy");
        count += 1;
    }
    CHECK(count == 2);
}

TEST_CASE("Test robin") {
    robin<uint8_t> RR(6U);
    auto count = 0U;
    for (auto _i : RR.exclude(2)) {
        static_assert(sizeof _i >= 0, "make compiler happy");
        count += 1;
    }
    CHECK(count == 5);
}
