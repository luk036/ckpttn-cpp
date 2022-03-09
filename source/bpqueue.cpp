#include <ckpttn/bpqueue.hpp>
#include <ckpttn/dllist.hpp>
#include <cstdint>  // for int32_t
/**
 * @brief sentinel
 *
 * @tparam T
 */
// template <typename T, typename Int, class Container>
// dllink<std::pair<T, Int>> bpqueue<T, Int, Container>::sentinel {};

template class bpqueue<int, int32_t>;
// template class bpqueue<int, int32_t,
//                FMPmr::vector<dllink<std::pair<int, int32_t>> > >;
