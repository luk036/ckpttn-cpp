#pragma once

#include <cassert>
#include <utility>  // for std::move()

// Forward declaration for begin() end()
template <typename T> class Dllist;
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
template <typename T> class Dllink {
    friend DllIterator<T>;
    friend Dllist<T>;
    friend DllIterator<T>;

  private:
    Dllink *next{this}; /**< pointer to the next node */
    Dllink *prev{this}; /**< pointer to the previous node */

  public:
    T data{}; /**< data */

    /**
     * @brief Construct a new Dllink object
     *
     * @param[in] data the data
     */
    constexpr explicit Dllink(T data) noexcept : data{std::move(data)} {
        static_assert(sizeof(Dllink) <= 24, "keep this class small");
    }

    /**
     * @brief Copy construct a new Dllink object (deleted intentionally)
     *
     */
    constexpr Dllink() = default;
    ~Dllink() = default;
    Dllink(const Dllink &) = delete;                                // don't copy
    constexpr auto operator=(const Dllink &) -> Dllink & = delete;  // don't assign
    constexpr Dllink(Dllink &&) noexcept = default;
    constexpr auto operator=(Dllink &&) noexcept -> Dllink & = default;  // don't assign

    /**
     * @brief lock the node (and don't append it to any list)
     *
     */
    constexpr auto lock() noexcept -> void { this->next = this; }

    /**
     * @brief whether the node is locked
     *
     * @return true
     * @return false
     */
    constexpr auto is_locked() const noexcept -> bool { return this->next == this; }


    /**
     * @brief detach from a list
     *
     */
    constexpr auto detach() noexcept -> void {
        assert(!this->is_locked());
        const auto n = this->next;
        const auto p = this->prev;
        p->next = n;
        n->prev = p;
    }

  private:
    /**
     * @brief append the node to the front
     *
     * @param[in,out] node
     */
    constexpr auto appendleft(Dllink &node) noexcept -> void {
        node.next = this->next;
        this->next->prev = &node;
        this->next = &node;
        node.prev = this;
    }

    /**
     * @brief append the node to the back
     *
     * @param[in,out] node
     */
    constexpr auto append(Dllink &node) noexcept -> void {
        node.prev = this->prev;
        this->prev->next = &node;
        this->prev = &node;
        node.next = this;
    }

    /**
     * @brief pop a node from the front
     *
     * @return Dllink&
     *
     * Precondition: list is not empty
     */
    constexpr auto popleft() noexcept -> Dllink & {
        auto res = this->next;
        this->next = res->next;
        this->next->prev = this;
        return *res;
    }

    /**
     * @brief pop a node from the back
     *
     * @return Dllink&
     *
     * Precondition: list is not empty
     */
    constexpr auto pop() noexcept -> Dllink & {
        auto res = this->prev;
        this->prev = res->prev;
        this->prev->next = this;
        return *res;
    }
};
#pragma pack(pop)

