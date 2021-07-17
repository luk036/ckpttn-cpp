#pragma once

#include <cassert>
// #include <cstdint>
#include <utility> // import std::move()

// Forward declaration for begin() end()
template <typename T>
class dll_iterator;

/*!
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
template <typename T>
class dllink
{
    friend dll_iterator<T>;

  private:
    dllink* next {this}; /*!< pointer to the next node */
    dllink* prev {this}; /*!< pointer to the previous node */

  public:
    T data {}; /*!< data */
    // Int key{}; /*!< key */

    /*!
     * @brief Construct a new dllink object
     *
     * @param[in] data the data
     */
    constexpr explicit dllink(T data) noexcept
        : data {std::move(data)}
    {
        static_assert(sizeof(dllink) <= 24, "keep this class small");
    }

    /*!
     * @brief Copy construct a new dllink object (deleted intentionally)
     *
     */
    constexpr dllink() = default;
    ~dllink() = default;
    dllink(const dllink&) = delete;                              // don't copy
    constexpr auto operator=(const dllink&) -> dllink& = delete; // don't assign
    constexpr dllink(dllink&&) noexcept = default;
    constexpr auto operator=(dllink&&) noexcept
        -> dllink& = default; // don't assign

    /*!
     * @brief lock the node (and don't append it to any list)
     *
     */
    constexpr auto lock() noexcept -> void
    {
        this->next = nullptr;
    }

    /*!
     * @brief whether the node is locked
     *
     * @return true
     * @return false
     */
    [[nodiscard]] constexpr auto is_locked() const noexcept -> bool
    {
        return this->next == nullptr;
    }

    /*!
     * @brief whether the list is empty
     *
     * @return true
     * @return false
     */
    [[nodiscard]] constexpr auto is_empty() const noexcept -> bool
    {
        return this->next == this;
    }

    /*!
     * @brief reset the list
     *
     */
    constexpr auto clear() noexcept -> void
    {
        this->next = this->prev = this;
    }

    /*!
     * @brief detach from a list
     *
     */
    constexpr auto detach() noexcept -> void
    {
        assert(!this->is_locked());
        const auto n = this->next;
        const auto p = this->prev;
        p->next = n;
        n->prev = p;
    }

    /*!
     * @brief append the node to the front
     *
     * @param[in,out] node
     */
    constexpr auto appendleft(dllink& node) noexcept -> void
    {
        node.next = this->next;
        this->next->prev = &node;
        this->next = &node;
        node.prev = this;
    }

    /*!
     * @brief append the node to the back
     *
     * @param[in,out] node
     */
    constexpr auto append(dllink& node) noexcept -> void
    {
        node.prev = this->prev;
        this->prev->next = &node;
        this->prev = &node;
        node.next = this;
    }

    /*!
     * @brief pop a node from the front
     *
     * @return dllink&
     *
     * Precondition: list is not empty
     */
    constexpr auto popleft() noexcept -> dllink&
    {
        auto res = this->next;
        this->next = res->next;
        this->next->prev = this;
        return *res;
    }

    /*!
     * @brief pop a node from the back
     *
     * @return dllink&
     *
     * Precondition: list is not empty
     */
    constexpr auto pop() noexcept -> dllink&
    {
        auto res = this->prev;
        this->prev = res->prev;
        this->prev->next = this;
        return *res;
    }

    // For iterator

    /*!
     * @brief
     *
     * @return dll_iterator
     */
    constexpr auto begin() noexcept -> dll_iterator<T>;

    /*!
     * @brief
     *
     * @return dll_iterator
     */
    constexpr auto end() noexcept -> dll_iterator<T>;

    // using coro_t = boost::coroutines2::coroutine<dllink&>;
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

/*!
 * @brief list iterator
 *
 * List iterator. Traverse the list from the first item. Usually it is
 * safe to attach/detach list items during the iterator is active.
 */
template <typename T>
class dll_iterator
{
  private:
    dllink<T>* cur; /*!< pointer to the current item */

  public:
    /*!
     * @brief Construct a new dll iterator object
     *
     * @param[in] cur
     */
    constexpr explicit dll_iterator(dllink<T>* cur) noexcept
        : cur {cur}
    {
    }

    /*!
     * @brief move to the next item
     *
     * @return dllink&
     */
    constexpr auto operator++() noexcept -> dll_iterator&
    {
        this->cur = this->cur->next;
        return *this;
    }

    /*!
     * @brief get the reference of the current item
     *
     * @return dllink&
     */
    constexpr auto operator*() noexcept -> dllink<T>&
    {
        return *this->cur;
    }

    /*!
     * @brief eq operator
     *
     * @param[in] rhs
     * @return true
     * @return false
     */
    friend auto operator==(
        const dll_iterator& lhs, const dll_iterator& rhs) noexcept -> bool
    {
        return lhs.cur == rhs.cur;
    }

    /*!
     * @brief neq operator
     *
     * @param[in] rhs
     * @return true
     * @return false
     */
    friend auto operator!=(
        const dll_iterator& lhs, const dll_iterator& rhs) noexcept -> bool
    {
        return !(lhs == rhs);
    }
};


/*!
 * @brief begin
 *
 * @return dll_iterator
 */
template <typename T>
inline constexpr auto dllink<T>::begin() noexcept -> dll_iterator<T>
{
    return dll_iterator<T> {this->next};
}

/*!
 * @brief end
 *
 * @return dll_iterator
 */
template <typename T>
inline constexpr auto dllink<T>::end() noexcept -> dll_iterator<T>
{
    return dll_iterator<T> {this};
}
