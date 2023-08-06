#pragma once

#include <cassert>
#include <utility>  // import std::move()

#include "dllink.hpp"  // for Dllink

// Forward declaration for begin() end()
template <typename T> class DllIterator;

/**
 * @brief doubly linked node (that may also be a "head" a list)
 *
 * A Doubly-linked List class. This class simply contains a link of
 * node's. By adding a "head" node (sentinel), deleting a node is
 * extremely fast (see "Introduction to Algorithm"). This class does
 * not keep the length information as it is not necessary for the FM
 * algorithm. This saves memory and run-time to update the length
 * information. Note that this class does not own the list node. They
 * are supplied by the caller in order to better reuse the nodes.
 */
#pragma pack(push, 1)
template <typename T> class Dllist {
    friend DllIterator<T>;

  private:
    Dllink<T> head;

  public:
    /**
     * @brief Construct a new Dllist object
     *
     * @param[in] data the data
     */
    constexpr explicit Dllist(T data) noexcept : head{std::move(data)} {
        static_assert(sizeof(Dllist) <= 24, "keep this class small");
    }

    /**
     * @brief Copy construct a new Dllist object (deleted intentionally)
     *
     */
    constexpr Dllist() = default;
    ~Dllist() = default;
    Dllist(const Dllist &) = delete;                                // don't copy
    constexpr auto operator=(const Dllist &) -> Dllist & = delete;  // don't assign
    constexpr Dllist(Dllist &&) noexcept = default;
    constexpr auto operator=(Dllist &&) noexcept -> Dllist & = default;  // don't assign

    /**
     * @brief whether the list is empty
     *
     * @return true
     * @return false
     */
    constexpr auto is_empty() const noexcept -> bool { return this->head.next == &this->head; }

    /**
     * @brief reset the list
     *
     */
    constexpr auto clear() noexcept -> void { this->head.next = this->head.prev = &this->head; }

    /**
     * @brief append the node to the front
     *
     * @param[in,out] node
     */
    constexpr auto appendleft(Dllink<T> &node) noexcept -> void { this->head.appendleft(node); }

    /**
     * @brief append the node to the back
     *
     * @param[in,out] node
     */
    constexpr auto append(Dllink<T> &node) noexcept -> void { this->head.append(node); }

    /**
     * @brief pop a node from the front
     *
     * @return Dllist&
     *
     * Precondition: list is not empty
     */
    constexpr auto popleft() noexcept -> Dllink<T> & { return this->head.popleft(); }

    /**
     * @brief pop a node from the back
     *
     * @return Dllist&
     *
     * Precondition: list is not empty
     */
    constexpr auto pop() noexcept -> Dllink<T> & { return this->head.pop(); }

    // For iterator

    /**
     * @brief
     *
     * @return DllIterator
     */
    constexpr auto begin() noexcept -> DllIterator<T>;

    /**
     * @brief
     *
     * @return DllIterator
     */
    constexpr auto end() noexcept -> DllIterator<T>;

    // using coro_t = boost::coroutines2::coroutine<Dllist&>;
    // using pull_t = typename coro_t::pull_type;

    // /**
    //  * @brief item generator
    //  *
    //  * @return pull_t
    //  */
    // auto items() noexcept -> pull_t
    // {
    //     auto func = [&](typename coro_t::push_type& yield) {
    //         auto cur = this->next;
    //         while (cur != this)
    //         {
    //             yield(*cur);
    //             cur = cur->next;
    //         }
    //     };
    //     return pull_t(func);
    // }
};
#pragma pack(pop)

/**
 * @brief list iterator
 *
 * List iterator. Traverse the list from the first item. Usually it is
 * safe to attach/detach list items during the iterator is active.
 */
template <typename T> class DllIterator {
  private:
    Dllink<T> *cur; /**< pointer to the current item */

  public:
    /**
     * @brief Construct a new dll iterator object
     *
     * @param[in] cur
     */
    constexpr explicit DllIterator(Dllink<T> *cur) noexcept : cur{cur} {}

    /**
     * @brief move to the next item
     *
     * @return Dllist&
     */
    constexpr auto operator++() noexcept -> DllIterator & {
        this->cur = this->cur->next;
        return *this;
    }

    /**
     * @brief get the reference of the current item
     *
     * @return Dllist&
     */
    constexpr auto operator*() noexcept -> Dllink<T> & { return *this->cur; }

    /**
     * @brief eq operator
     *
     * @param[in] lhs
     * @param[in] rhs
     * @return true
     * @return false
     */
    friend auto operator==(const DllIterator &lhs, const DllIterator &rhs) noexcept -> bool {
        return lhs.cur == rhs.cur;
    }

    /**
     * @brief neq operator
     *
     * @param[in] lhs
     * @param[in] rhs
     * @return true
     * @return false
     */
    friend auto operator!=(const DllIterator &lhs, const DllIterator &rhs) noexcept -> bool {
        return !(lhs == rhs);
    }
};

/**
 * @brief begin
 *
 * @return DllIterator
 */
template <typename T> inline constexpr auto Dllist<T>::begin() noexcept -> DllIterator<T> {
    return DllIterator<T>{this->head.next};
}

/**
 * @brief end
 *
 * @return DllIterator
 */
template <typename T> inline constexpr auto Dllist<T>::end() noexcept -> DllIterator<T> {
    return DllIterator<T>{&this->head};
}
