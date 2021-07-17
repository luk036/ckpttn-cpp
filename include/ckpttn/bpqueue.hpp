#pragma once

#include "dllist.hpp" // import dllink
#include <cassert>
#include <gsl/span>
// #include <type_traits>
#include <vector>

// Forward declaration for begin() end()
template <typename _Tp, typename Int>
class bpq_iterator;

/*!
 * @brief bounded priority queue
 *
 * Bounded Priority Queue with integer keys in [a..b].
 * Implemented by array (bucket) of doubly-linked lists.
 * Efficient if key is bounded by a small integer value.
 *
 * Note that this class does not own the PQ nodes. This feature
 * makes the nodes sharable between doubly linked list class and
 * this class. In the FM algorithm, the node either attached to
 * the gain buckets (PQ) or in the waitinglist (doubly linked list),
 * but not in both of them in the same time.
 *
 * Another improvement is to make the array size one element bigger
 * i.e. (b - a + 2). The extra dummy array element (which is called
 * sentinel) is used to reduce the boundary checking during updating.
 *
 * All the member functions assume that the keys are within the bound.
 *
 * @TODO: support std::pmr
 */
template <typename _Tp, typename Int = int32_t,
    typename _Sequence = std::vector<dllink<std::pair<_Tp, std::make_unsigned_t<Int>>>>>
//          class Allocator = typename std::allocator<dllink<std::pair<_Tp,
//          Int>> > >
class bpqueue
{
    using UInt = std::make_unsigned_t<Int>;
    
    friend bpq_iterator<_Tp, Int>;
    using Item = dllink<std::pair<_Tp, UInt>>;

    static_assert(std::is_same_v<Item, typename _Sequence::value_type>,
        "value_type must be the same as the underlying container");

  public:
    using value_type = typename _Sequence::value_type;
    using reference = typename _Sequence::reference;
    using const_reference = typename _Sequence::const_reference;
    using size_type = typename _Sequence::size_type;
    using container_type = _Sequence;

  private:
    Item sentinel {}; //!< sentinel */

    _Sequence bucket; //!< bucket, array of lists
    UInt max {};       //!< max value
    Int offset;          //!< a - 1
    UInt high;         //!< b - a + 1

    // using alloc_t = decltype(bucket.get_allocator());

  public:
    /*!
     * @brief Construct a new bpqueue object
     *
     * @param[in] a lower bound
     * @param[in] b upper bound
     */
    constexpr bpqueue(Int a, Int b)
        : bucket(static_cast<UInt>(b - a) + 2U)
        , offset(a - 1)
        , high(static_cast<UInt>(b - offset))
    {
        assert(a <= b);
        static_assert(
            std::is_integral<Int>::value, "bucket's key must be an integer");
        bucket[0].append(this->sentinel); // sentinel
    }

    bpqueue(const bpqueue&) = delete; // don't copy
    ~bpqueue() = default;
    constexpr auto operator=(const bpqueue&)
        -> bpqueue& = delete; // don't assign
    constexpr bpqueue(bpqueue&&) noexcept = default;
    constexpr auto operator=(bpqueue&&) noexcept
        -> bpqueue& = default; // don't assign

    /*!
     * @brief whether the %bpqueue is empty.
     *
     * @return true
     * @return false
     */
    [[nodiscard]] constexpr auto is_empty() const noexcept -> bool
    {
        return this->max == 0U;
    }

    /*!
     * @brief Set the key object
     *
     * @param[out] it   the item
     * @param[in] gain the key of it
     */
    constexpr auto set_key(Item& it, Int gain) noexcept -> void
    {
        it.data.second = static_cast<UInt>(gain - this->offset);
    }

    /*!
     * @brief Get the max value
     *
     * @return T maximum value
     */
    [[nodiscard]] constexpr auto get_max() const noexcept -> Int
    {
        return this->offset + Int(this->max);
    }

    /*!
     * @brief clear reset the PQ
     */
    constexpr auto clear() noexcept -> void
    {
        while (this->max > 0)
        {
            this->bucket[this->max].clear();
            this->max -= 1;
        }
    }

    /*!
     * @brief append item with internal key
     *
     * @param[in,out] it the item
     */
    constexpr auto append_direct(Item& it) noexcept -> void
    {
        assert(static_cast<Int>(it.data.second) > this->offset);
        this->append(it, Int(it.data.second));
    }

    /*!
     * @brief append item with external key
     *
     * @param[in,out] it the item
     * @param[in] k  the key
     */
    constexpr auto append(Item& it, Int k) noexcept -> void
    {
        assert(k > this->offset);
        it.data.second = UInt(k - this->offset);
        if (this->max < it.data.second)
        {
            this->max = it.data.second;
        }
        this->bucket[it.data.second].append(it);
    }

    /*!
     * @brief append from list
     *
     * @param[in,out] nodes
     */
    // constexpr auto appendfrom(gsl::span<Item> nodes) noexcept -> void
    // {
    //     for (auto& it : nodes)
    //     {
    //         it.data.second -= this->offset;
    //         assert(it.data.second > 0);
    //         this->bucket[it.data.second].append(it);
    //     }
    //     this->max = this->high;
    //     while (this->bucket[this->max].is_empty())
    //     {
    //         this->max -= 1;
    //     }
    // }

    /*!
     * @brief pop node with the highest key
     *
     * @return dllink&
     */
    constexpr auto popleft() noexcept -> Item&
    {
        auto& res = this->bucket[this->max].popleft();
        while (this->bucket[this->max].is_empty())
        {
            this->max -= 1;
        }
        return res;
    }

    /*!
     * @brief decrease key by delta
     *
     * @param[in,out] it the item
     * @param[in] delta the change of the key
     *
     * Note that the order of items with same key will not be preserved.
     * For FM algorithm, this is a prefered behavior.
     */
    constexpr auto decrease_key(Item& it, UInt delta) noexcept -> void
    {
        // this->bucket[it.data.second].detach(it)
        it.detach();
        it.data.second -= delta;
        assert(it.data.second > 0);
        assert(it.data.second <= this->high);
        this->bucket[it.data.second].append(it); // FIFO
        if (this->max < it.data.second)
        {
            this->max = it.data.second;
            return;
        }
        while (this->bucket[this->max].is_empty())
        {
            this->max -= 1;
        }
    }

    /*!
     * @brief increase key by delta
     *
     * @param[in,out] it the item
     * @param[in] delta the change of the key
     *
     * Note that the order of items with same key will not be preserved.
     * For FM algorithm, this is a prefered behavior.
     */
    constexpr auto increase_key(Item& it, UInt delta) noexcept -> void
    {
        // this->bucket[it.data.second].detach(it)
        it.detach();
        it.data.second += delta;
        assert(it.data.second > 0);
        assert(it.data.second <= this->high);
        this->bucket[it.data.second].appendleft(it); // LIFO
        if (this->max < it.data.second)
        {
            this->max = it.data.second;
        }
    }

    /*!
     * @brief modify key by delta
     *
     * @param[in,out] it the item
     * @param[in] delta the change of the key
     *
     * Note that the order of items with same key will not be preserved.
     * For FM algorithm, this is a prefered behavior.
     */
    constexpr auto modify_key(Item& it, Int delta) noexcept -> void
    {
        if (it.is_locked())
        {
            return;
        }
        if (delta > 0)
        {
            this->increase_key(it, UInt(delta));
        }
        else if (delta < 0)
        {
            this->decrease_key(it, UInt(-delta));
        }
    }

    /*!
     * @brief detach the item from bpqueue
     *
     * @param[in,out] it the item
     */
    constexpr auto detach(Item& it) noexcept -> void
    {
        // this->bucket[it.data.second].detach(it)
        it.detach();
        while (this->bucket[this->max].is_empty())
        {
            this->max -= 1;
        }
    }

    /*!
     * @brief iterator point to begin
     *
     * @return bpq_iterator
     */
    constexpr auto begin() -> bpq_iterator<_Tp, Int>;

    /*!
     * @brief iterator point to end
     *
     * @return bpq_iterator
     */
    constexpr auto end() -> bpq_iterator<_Tp, Int>;

    // constexpr auto& items()
    // {
    //     return *this;
    // }

    // constexpr const auto& items() const
    // {
    //     return *this;
    // }

    // using coro_t = boost::coroutines2::coroutine<dllink<T>&>;
    // using pull_t = typename coro_t::pull_type;

    // /**
    //  * @brief item generator
    //  *
    //  * @return pull_t
    //  */
    // auto items() -> pull_t
    // {
    //     auto func = [&](typename coro_t::push_type& yield) {
    //         auto curkey = this->max;
    //         while (curkey > 0)
    //         {
    //             for (const auto& item : this->bucket[curkey].items())
    //             {
    //                 yield(item);
    //             }
    //             curkey -= 1;
    //         }
    //     };
    //     return pull_t(func);
    // }
};

/*!
 * @brief Bounded Priority Queue Iterator
 *
 * Bounded Priority Queue Iterator. Traverse the queue in descending
 * order. Detaching queue items may invalidate the iterator because
 * the iterator makes a copy of current key.
 */
template <typename _Tp, typename Int = int32_t>
class bpq_iterator
{
    using UInt = std::make_unsigned_t<Int>;

    // using value_type = _Tp;
    // using key_type = Int;
    using Item = dllink<std::pair<_Tp, UInt>>;

  private:
    bpqueue<_Tp, Int>& bpq; /*!< the priority queue */
    UInt curkey;             /*!< the current key value */
    dll_iterator<std::pair<_Tp, UInt>>
        curitem; /*!< list iterator pointed to the current item.
                  */

    /*!
     * @brief get the reference of the current list
     *
     * @return dllink&
     */
    constexpr auto curlist() -> Item&
    {
        return this->bpq.bucket[this->curkey];
    }

  public:
    /*!
     * @brief Construct a new bpq iterator object
     *
     * @param[in] bpq
     * @param[in] curkey
     */
    constexpr bpq_iterator(bpqueue<_Tp, Int>& bpq, UInt curkey)
        : bpq {bpq}
        , curkey {curkey}
        , curitem {bpq.bucket[curkey].begin()}
    {
    }

    /*!
     * @brief move to the next item
     *
     * @return bpq_iterator&
     */
    constexpr auto operator++() -> bpq_iterator&
    {
        ++this->curitem;
        while (this->curitem == this->curlist().end())
        {
            do
            {
                this->curkey -= 1;
            } while (this->curlist().is_empty());
            this->curitem = this->curlist().begin();
        }
        return *this;
    }

    /*!
     * @brief get the reference of the current item
     *
     * @return bpq_iterator&
     */
    constexpr auto operator*() -> Item&
    {
        return *this->curitem;
    }

    /*!
     * @brief eq operator
     *
     * @param[in] rhs
     * @return true
     * @return false
     */
    friend constexpr auto operator==(
        const bpq_iterator& lhs, const bpq_iterator& rhs) -> bool
    {
        return lhs.curitem == rhs.curitem;
    }

    /*!
     * @brief neq operator
     *
     * @param[in] rhs
     * @return true
     * @return false
     */
    friend constexpr auto operator!=(
        const bpq_iterator& lhs, const bpq_iterator& rhs) -> bool
    {
        return !(lhs == rhs);
    }
};

/*!
 * @brief
 *
 * @return bpq_iterator
 */
template <typename _Tp, typename Int, class _Sequence>
inline constexpr auto bpqueue<_Tp, Int, _Sequence>::begin()
    -> bpq_iterator<_Tp, Int>
{
    return {*this, this->max};
}

/*!
 * @brief
 *
 * @return bpq_iterator
 */
template <typename _Tp, typename Int, class _Sequence>
inline constexpr auto bpqueue<_Tp, Int, _Sequence>::end()
    -> bpq_iterator<_Tp, Int>
{
    return {*this, 0};
}
