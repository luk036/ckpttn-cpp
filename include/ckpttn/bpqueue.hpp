#pragma once

#include <cassert>      // for assert
#include <cstdint>      // for int32_t
#include <type_traits>  // for make_unsigned_t, is_integral, integral_consta...
#include <utility>      // for pair
#include <vector>       // for vector, vector<>::value_type, vector<>::const...

#include "dllist.hpp"  // for Dllink, DllIterator

// Forward declaration for begin() end()
template <typename Tp, typename Int> class BpqIterator;

/**
 * @brief Bounded priority queue
 *
 * Bounded Priority Queue with integer keys in [a..b].
 * Implemented by an array (bucket) of doubly-linked lists.
 * Efficient if the keys are bounded by a small integer value.
 *
 * Note that this class does not own PQ nodes. This feature
 * allows these nodes sharable in both doubly linked list class and
 * this class. In the Fiduccia-Mattheyses algorithm, nodes are either attached to
 * the gain buckets (PQ) or to the waiting_list (doubly-linked list),
 * but cannot be in both at the same time.
 *
 * Another improvement is to increase the size of the array by one
 * element, i.e. (b - a + 2). The extra dummy array element (called
 * sentinel) is used to reduce the boundary checking during updates.
 *
 * All the member functions assume that the keys are inside the bounds.
 *
 * @tparam Tp
 * @tparam Int
 * @tparam _Sequence
 * @tparam std::make_unsigned_t<Int>>>>
 */
template <typename Tp, typename Int = int32_t,
          typename Sequence = std::vector<Dllist<std::pair<Tp, std::make_unsigned_t<Int>>>>>
class BPQueue {
    using UInt = std::make_unsigned_t<Int>;

    friend BpqIterator<Tp, Int>;
    using Item = Dllink<std::pair<Tp, UInt>>;

    // static_assert(std::is_same<Item, typename _Sequence::value_type>::value,
    //               "value_type must be the same as the underlying container");

  public:
    using value_type = typename Sequence::value_type;
    using reference = typename Sequence::reference;
    using const_reference = typename Sequence::const_reference;
    using size_type = typename Sequence::size_type;
    using container_type = Sequence;

  private:
    Item sentinel{};  //!< sentinel */
    Sequence bucket;  //!< bucket, array of lists
    UInt max{};       //!< max value
    Int offset;       //!< a - 1
    UInt high;        //!< b - a + 1

  public:
    /**
     * @brief Construct a new BPQueue object
     *
     * @param[in] a lower bound
     * @param[in] b upper bound
     */
    constexpr BPQueue(Int a, Int b)
        : bucket(static_cast<UInt>(b - a) + 2U),
          offset(a - 1),
          high(static_cast<UInt>(b - offset)) {
        assert(a <= b);
        static_assert(std::is_integral<Int>::value, "bucket's key must be an integer");
        bucket[0].append(this->sentinel);  // sentinel
    }

    BPQueue(const BPQueue &) = delete;  // don't copy
    ~BPQueue() = default;
    constexpr auto operator=(const BPQueue &) -> BPQueue & = delete;  // don't assign
    constexpr BPQueue(BPQueue &&) noexcept = default;
    constexpr auto operator=(BPQueue &&) noexcept -> BPQueue & = default;  // don't assign

    /**
     * @brief Whether the %BPQueue is empty.
     *
     * @return true
     * @return false
     */
    constexpr auto is_empty() const noexcept -> bool { return this->max == 0U; }

    /**
     * @brief Set the key object
     *
     * @param[out] it the item
     * @param[in] gain the key of it
     */
    constexpr auto set_key(Item &it, Int gain) noexcept -> void {
        it.data.second = static_cast<UInt>(gain - this->offset);
    }

    /**
     * @brief Get the max value
     *
     * @return Int maximum value
     */
    constexpr auto get_max() const noexcept -> Int { return this->offset + Int(this->max); }

    /**
     * @brief Clear reset the PQ
     */
    constexpr auto clear() noexcept -> void {
        while (this->max > 0) {
            this->bucket[this->max].clear();
            this->max -= 1;
        }
    }

    /**
     * @brief Append item with internal key
     *
     * @param[in,out] it the item
     */
    constexpr auto append_direct(Item &it) noexcept -> void {
        assert(static_cast<Int>(it.data.second) > this->offset);
        this->append(it, Int(it.data.second));
    }

    /**
     * @brief Append item with external key
     *
     * @param[in,out] it the item
     * @param[in] k  the key
     */
    constexpr auto append(Item &it, Int k) noexcept -> void {
        assert(k > this->offset);
        it.data.second = UInt(k - this->offset);
        if (this->max < it.data.second) {
            this->max = it.data.second;
        }
        this->bucket[it.data.second].append(it);
    }

    /**
     * @brief Pop node with the highest key
     *
     * @return Dllink&
     */
    constexpr auto popleft() noexcept -> Item & {
        auto &res = this->bucket[this->max].popleft();
        while (this->bucket[this->max].is_empty()) {
            this->max -= 1;
        }
        return res;
    }

    /**
     * @brief Decrease key by delta
     *
     * @param[in,out] it the item
     * @param[in] delta the change of the key
     *
     * Note that the order of items with same key will not be preserved.
     * For the Fiduccia-Mattheyses algorithm, this is a prefered behavior.
     */
    constexpr auto decrease_key(Item &it, UInt delta) noexcept -> void {
        // this->bucket[it.data.second].detach(it)
        it.detach();
        it.data.second -= delta;
        assert(it.data.second > 0);
        assert(it.data.second <= this->high);
        this->bucket[it.data.second].append(it);  // FIFO
        if (this->max < it.data.second) {
            this->max = it.data.second;
            return;
        }
        while (this->bucket[this->max].is_empty()) {
            this->max -= 1;
        }
    }

    /**
     * @brief Increase key by delta
     *
     * @param[in,out] it the item
     * @param[in] delta the change of the key
     *
     * Note that the order of items with same key will not be preserved.
     * For the Fiduccia-Mattheyses algorithm, this is a prefered behavior.
     */
    constexpr auto increase_key(Item &it, UInt delta) noexcept -> void {
        // this->bucket[it.data.second].detach(it)
        it.detach();
        it.data.second += delta;
        assert(it.data.second > 0);
        assert(it.data.second <= this->high);
        this->bucket[it.data.second].appendleft(it);  // LIFO
        if (this->max < it.data.second) {
            this->max = it.data.second;
        }
    }

    /**
     * @brief Modify key by delta
     *
     * @param[in,out] it the item
     * @param[in] delta the change of the key
     *
     * Note that the order of items with same key will not be preserved.
     * For Fiduccia-Mattheyses algorithm, this is a prefered behavior.
     */
    constexpr auto modify_key(Item &it, Int delta) noexcept -> void {
        if (it.is_locked()) {
            return;
        }
        if (delta > 0) {
            this->increase_key(it, UInt(delta));
        } else if (delta < 0) {
            this->decrease_key(it, UInt(-delta));
        }
    }

    /**
     * @brief Detach the item from BPQueue
     *
     * @param[in,out] it the item
     */
    constexpr auto detach(Item &it) noexcept -> void {
        // this->bucket[it.data.second].detach(it)
        it.detach();
        while (this->bucket[this->max].is_empty()) {
            this->max -= 1;
        }
    }

    /**
     * @brief Iterator point to the begin
     *
     * @return BpqIterator
     */
    constexpr auto begin() -> BpqIterator<Tp, Int>;

    /**
     * @brief Iterator point to the end
     *
     * @return BpqIterator
     */
    constexpr auto end() -> BpqIterator<Tp, Int>;
};

/**
 * @brief Bounded Priority Queue Iterator
 *
 * Traverse the queue in descending order.
 * Detaching a queue items may invalidate the iterator because
 * the iterator makes a copy of the current key.
 */
template <typename Tp, typename Int = int32_t> class BpqIterator {
    using UInt = std::make_unsigned_t<Int>;

    // using value_type = Tp;
    // using key_type = Int;
    using Item = Dllink<std::pair<Tp, UInt>>;

  private:
    BPQueue<Tp, Int> &bpq;                     //!< the priority queue
    UInt curkey;                               //!< the current key value
    DllIterator<std::pair<Tp, UInt>> curitem;  //!< list iterator pointed to the current item.

    /**
     * @brief Get the reference of the current list
     *
     * @return Item&
     */
    constexpr auto curlist() -> Item & { return this->bpq.bucket[this->curkey]; }

  public:
    /**
     * @brief Construct a new bpq iterator object
     *
     * @param[in] bpq
     * @param[in] curkey
     */
    constexpr BpqIterator(BPQueue<Tp, Int> &bpq, UInt curkey)
        : bpq{bpq}, curkey{curkey}, curitem{bpq.bucket[curkey].begin()} {}

    /**
     * @brief Move to the next item
     *
     * @return BpqIterator&
     */
    constexpr auto operator++() -> BpqIterator & {
        ++this->curitem;
        while (this->curitem == this->curlist().end()) {
            do {
                this->curkey -= 1;
            } while (this->curlist().is_empty());
            this->curitem = this->curlist().begin();
        }
        return *this;
    }

    /**
     * @brief Get the reference of the current item
     *
     * @return Item&
     */
    constexpr auto operator*() -> Item & { return *this->curitem; }

    /**
     * @brief eq operator
     *
     * @param[in] lhs
     * @param[in] rhs
     * @return true
     * @return false
     */
    friend constexpr auto operator==(const BpqIterator &lhs, const BpqIterator &rhs) -> bool {
        return lhs.curitem == rhs.curitem;
    }

    /**
     * @brief neq operator
     *
     * @param[in] lhs
     * @param[in] rhs
     * @return true
     * @return false
     */
    friend constexpr auto operator!=(const BpqIterator &lhs, const BpqIterator &rhs) -> bool {
        return !(lhs == rhs);
    }
};

/**
 * @brief
 *
 * @return BpqIterator
 */
template <typename Tp, typename Int, class Sequence>
inline constexpr auto BPQueue<Tp, Int, Sequence>::begin() -> BpqIterator<Tp, Int> {
    return {*this, this->max};
}

/**
 * @brief
 *
 * @return BpqIterator
 */
template <typename Tp, typename Int, class Sequence>
inline constexpr auto BPQueue<Tp, Int, Sequence>::end() -> BpqIterator<Tp, Int> {
    return {*this, 0};
}
