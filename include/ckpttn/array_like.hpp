#pragma once

// #include <any>
#include <cassert>
#include <cstddef>
#include <utility>
// #include <range/v3/view/repeat_n.hpp>
// #include <type_traits>

// template <typename Val> inline auto get_repeat_array(const Val& a,
// std::ptrdiff_t n) {
//     using repeat_n_return_type = decltype(ranges::views::repeat_n(a, n));

//     struct iterable_wrapper : public repeat_n_return_type {
//       public:
//         using value_type [[maybe_unused]] = Val;   // luk:
//         using key_type [[maybe_unused]] = size_t;  // luk:

//         iterable_wrapper(repeat_n_return_type&& base)
//             : repeat_n_return_type{std::forward<repeat_n_return_type>(base)}
//             {}

//         auto operator[](const std::any& /* don't care */) const -> const Val&
//         {
//             return *this->begin();
//         }
//     };

//     return iterable_wrapper{ranges::views::repeat_n(a, n)};
// }

/**
 * @brief
 *
 * @tparam Container
 */

/**
 * @brief Shift Array
 *
 * The `ShiftArray` class is a template class that extends a given container
 * type. It allows accessing elements of the container using shifted indices.
 * The shift value is set using the `set_start` method, and the shifted indices
 * are calculated by subtracting the shift value from the original index. The
 * class provides `operator[]` overloads to access elements using shifted
 * indices.
 *
 * @tparam Container
 */
template <typename Container> class ShiftArray : public Container {
    using value_type = typename Container::value_type;

  private:
    size_t _start{0U};

  public:
    /**
     * @brief Construct a new Shift Array object
     *
     */
    ShiftArray() : Container{} {}

    /**
     * @brief Construct a new Shift Array object
     *
     * @param[in] base
     */
    explicit ShiftArray(Container &&base) : Container{std::forward<Container>(base)} {}

    /**
     * @brief Set the start object
     *
     * @param[in] start
     */
    void set_start(const size_t &start) { this->_start = start; }

    /**
     * @brief
     *
     * @param[in] index
     * @return const value_type&
     */
    auto operator[](const size_t &index) const -> const value_type & {
        assert(index >= this->_start);
        return Container::operator[](index - this->_start);
    }

    /**
     * @brief
     *
     * @param[in] index
     * @return value_type&
     */
    auto operator[](const size_t &index) -> value_type & {
        return Container::operator[](index - this->_start);
    }
};
