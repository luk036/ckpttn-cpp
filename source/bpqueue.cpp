#include <ckpttn/bpqueue.hpp>
#include <ckpttn/dllist.hpp>
#include <cstdint>  // for int32_t
/**
 * @brief sentinel
 *
 * @tparam T
 */
// template <typename T, typename Int, class Container>
// Dllink<std::pair<T, Int>> BPQueue<T, Int, Container>::sentinel ;

template class BPQueue<int, int32_t>;
// template class BPQueue<int, int32_t,
//                FMPmr::vector<Dllink<std::pair<int, int32_t>> > >;
