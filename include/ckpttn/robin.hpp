#pragma once

#include <vector>

namespace fun {

    namespace detail {

        /**
         * @brief
         *
         * @tparam T
         */
        template <typename T> struct RobinSlNode {
            RobinSlNode *next;
            T key;
        };

        /**
         * @brief
         *
         * @tparam T
         */
        template <typename T> struct RobinIterator {
            const RobinSlNode<T> *cur;

            /**
             * @brief
             *
             * @param other
             * @return true
             * @return false
             */
            auto operator!=(const RobinIterator &other) const -> bool { return cur != other.cur; }

            /**
             * @brief
             *
             * @param other
             * @return true
             * @return false
             */
            auto operator==(const RobinIterator &other) const -> bool { return cur == other.cur; }

            /**
             * @brief
             *
             * @return RobinIterator&
             */
            auto operator++() -> RobinIterator & {
                cur = cur->next;
                return *this;
            }

            /**
             * @brief
             *
             * @return const T&
             */
            auto operator*() const -> const T & { return cur->key; }
        };

        /**
         * @brief
         *
         * @tparam T
         */
        template <typename T> struct RobinIterableWrapper {
            const detail::RobinSlNode<T> *node;
            // const Robin<T> *rr;
            // T from_part;

            /**
             * @brief
             *
             * @return RobinIterator<T>
             */
            auto begin() const -> RobinIterator<T> { return RobinIterator<T>{node->next}; }

            /**
             * @brief
             *
             * @return RobinIterator<T>
             */
            auto end() const -> RobinIterator<T> { return RobinIterator<T>{node}; }
            // auto size() const -> size_t { return rr->cycle.size() - 1; }
        };
    }  // namespace detail

    /**
     * @brief Round Robin
     *
     * The `Robin` class is implementing a round-robin algorithm. It is used to
     * cycle through a sequence of elements in a circular manner. The constructor
     * initializes the cycle with a specified number of parts, and each part is
     * assigned a unique key. The `exclude` method returns an iterable wrapper that
     * excludes a specified part from the cycle.
     *
     * @tparam T
     */
    template <typename T> struct Robin {
        std::vector<detail::RobinSlNode<T>> cycle;

        /**
         * @brief Construct a new Robin object
         *
         * @param num_parts
         */
        explicit Robin(T num_parts) : cycle(num_parts) {
            auto *slptr = &this->cycle[num_parts - 1];
            auto k = T(0);
            for (auto &sl : this->cycle) {
                sl.key = k;
                slptr->next = &sl;
                slptr = slptr->next;
                ++k;
            }
        }

        /**
         * @brief exclude
         *
         * The `exclude` method in the `Robin` class returns an iterable wrapper
         * that excludes a specified part from the cycle.
         *
         * @param from_part
         * @return detail::RobinIterableWrapper<T>
         */
        auto exclude(T from_part) const -> detail::RobinIterableWrapper<T> {
            return detail::RobinIterableWrapper<T>{&this->cycle[from_part]};
        }
    };

}  // namespace fun
