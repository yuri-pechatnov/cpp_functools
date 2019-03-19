#pragma once

#include <reverse.h>
#include <xrange.h>
#include <mapped.h>
#include <filtering.h>

#include <tuple>

#define Y_LIKELY(Cond) __builtin_expect(!!(Cond), 1)
#define Y_UNLIKELY(Cond) __builtin_expect(!!(Cond), 0)

namespace NFuncToolsPrivate {

    template <typename TContainer>
    struct TEnumerator {
    private:
        using TStorage = TAutoEmbedOrPtrPolicy<TContainer>;
        using TValue = std::tuple<const std::size_t&, decltype(*std::begin(std::declval<TContainer&>()))>;
        using TIteratorState = decltype(std::begin(std::declval<TContainer&>()));
        using TSentinelState = decltype(std::end(std::declval<TContainer&>()));

        struct TIterator;
        struct TSentinelCandidate {
            std::size_t Index_;
            TSentinelState Iterator_;
        };
        using TSentinel = std::conditional_t<std::is_same_v<TIteratorState, TSentinelState>,
                                             TIterator, TSentinelCandidate>;

        struct TIterator {
            using difference_type = std::ptrdiff_t;
            using value_type = TValue;
            using pointer = TValue*;
            using reference = TValue&;
            using iterator_category = std::input_iterator_tag;

            TValue operator*() {
                return {Index_, *Iterator_};
            }
            TValue operator*() const {
                return {Index_, *Iterator_};
            }
            void operator++() {
                ++Index_;
                ++Iterator_;
            }
            bool operator!=(const TSentinel& other) const {
                return Iterator_ != other.Iterator_;
            }
            bool operator==(const TSentinel& other) const {
                return Iterator_ == other.Iterator_;
            }

            std::size_t Index_;
            TIteratorState Iterator_;
        };
    public:
        using iterator = TIterator;
        using const_iterator = TIterator;

        TIterator begin() const {
            return {0, std::begin(*Storage_.Ptr())};
        }

        TSentinel end() const {
            return {std::numeric_limits<std::size_t>::max(), std::end(*Storage_.Ptr())};
        }

        mutable TStorage Storage_;
    };


    template <typename TContainer, typename TIteratorCategory = typename std::iterator_traits<decltype(std::begin(std::declval<TContainer>()))>::iterator_category>
    static constexpr bool HasRandomAccessIterator(int32_t) {
        return std::is_same_v<TIteratorCategory, std::random_access_iterator_tag>;
    }

    template <typename TContainer>
    static constexpr bool HasRandomAccessIterator(uint32_t) {
        return false;
    }

    template <typename... TContainers>
    struct TZipper {
        template <std::size_t... I>
        struct TZipperWithIndex {
        private:
            using THolders = std::tuple<TAutoEmbedOrPtrPolicy<TContainers>...>;
            using TValue = std::tuple<decltype(*std::begin(std::declval<TContainers&>()))...>;
            using TIteratorState = std::tuple<decltype(std::begin(std::declval<TContainers&>()))...>;
            using TSentinelState = std::tuple<decltype(std::end(std::declval<TContainers&>()))...>;

            static constexpr bool TrivialSentinel = std::is_same_v<TIteratorState, TSentinelState>;

            struct TIterator;
            struct TSentinelCandidate {
                TSentinelState Iterators_;
            };
            using TSentinel = std::conditional_t<TrivialSentinel, TIterator, TSentinelCandidate>;

#ifndef _WINDOWS
            // windows compiler crashes here
            static constexpr bool LimitByFirstContainer = TrivialSentinel &&
                ((HasRandomAccessIterator<TContainers>(0)) && ...);
#else
            static constexpr bool LimitByFirstContainer = false;
#endif

            struct TIterator {
                using difference_type = std::ptrdiff_t;
                using value_type = TValue;
                using pointer = TValue*;
                using reference = TValue&;
                using iterator_category = std::input_iterator_tag;

                TValue operator*() {
                    return {*std::get<I>(Iterators_)...};
                }
                void operator++() {
                    (++std::get<I>(Iterators_), ...);
                }
                bool operator!=(const TSentinel& other) const {
                    if constexpr (LimitByFirstContainer) {
                        return std::get<0>(Iterators_) != std::get<0>(other.Iterators_);
                    } else {
                        // yes, for all correct iterators but end() it is a correct way to compare
                        return ((std::get<I>(Iterators_) != std::get<I>(other.Iterators_)) && ...);
                    }
                }
                bool operator==(const TSentinel& other) const {
                    return !(*this != other);
                }

                TIteratorState Iterators_;
            };
        public:
            using iterator = TIterator;
            using const_iterator = TIterator;

            TIterator begin() const {
                return {TIteratorState{std::begin(*std::get<I>(Holders_).Ptr())...}};
            }

            TSentinel end() const {
                if constexpr (LimitByFirstContainer) {
                    auto endOfFirst = std::begin(*std::get<0>(Holders_).Ptr()) + std::min({
                        std::end(*std::get<I>(Holders_).Ptr()) - std::begin(*std::get<I>(Holders_).Ptr())...});
                    TIterator iter{TSentinelState{std::end(*std::get<I>(Holders_).Ptr())...}};
                    std::get<0>(iter.Iterators_) = endOfFirst;
                    return iter;
                } else {
                    return {TSentinelState{std::end(*std::get<I>(Holders_).Ptr())...}};
                }
            }

            mutable THolders Holders_;
        };

        template <std::size_t... I>
        static auto Zip(TContainers&&... containers, std::index_sequence<I...>) {
            return TZipperWithIndex<I...>{{std::forward<TContainers>(containers)...}};
        }
    };


    template <typename... TContainers>
    struct TCartesianMultiplier {
        template <std::size_t... I>
        struct TCartesianMultiplierWithIndex {
        private:
            using THolders = std::tuple<TAutoEmbedOrPtrPolicy<TContainers>...>;
            using TValue = std::tuple<decltype(*std::begin(std::declval<TContainers&>()))...>;
            using TIteratorState = std::tuple<int, decltype(std::begin(std::declval<TContainers&>()))...>;
            using TSentinelState = std::tuple<int, decltype(std::end(std::declval<TContainers&>()))...>;

            struct TIterator;
            struct TSentinelCandidate {
                TSentinelState Iterators_;
                THolders* HoldersPtr_;
            };
            using TSentinel = std::conditional_t<std::is_same_v<TIteratorState, TSentinelState>,
                                                 TIterator, TSentinelCandidate>;

            struct TIterator {
            private:
                //! Return value is true when iteration is not finished
                template <std::size_t position = sizeof...(TContainers)>
                void IncrementIteratorsTuple() {
                    auto& currentIterator = std::get<position>(Iterators_);
                    ++currentIterator;

                    if (currentIterator != std::end(*std::get<position - 1>(*HoldersPtr_).Ptr())) {
                        return;
                    } else {
                        currentIterator = std::begin(*std::get<position - 1>(*HoldersPtr_).Ptr());
                        if constexpr (position != 1) {
                            IncrementIteratorsTuple<position - 1>();
                        } else {
                            std::get<0>(Iterators_) = 1;
                        }
                    }
                }
            public:
                using difference_type = std::ptrdiff_t;
                using value_type = TValue;
                using pointer = TValue*;
                using reference = TValue&;
                using iterator_category = std::input_iterator_tag;

                TValue operator*() {
                    return {*std::get<I + 1>(Iterators_)...};
                }
                void operator++() {
                    IncrementIteratorsTuple();
                }
                bool operator!=(const TSentinel& other) const {
                    // not finished iterator VS sentinel (most frequent case)
                    if (std::get<0>(Iterators_) != std::get<0>(other.Iterators_)) {
                        return true;
                    }
                    // do not compare sentinels and finished iterators
                    if (std::get<0>(other.Iterators_)) {
                        return false;
                    }
                    // compare not finished iterators
                    return ((std::get<I + 1>(Iterators_) != std::get<I + 1>(other.Iterators_)) || ...);
                }
                bool operator==(const TSentinel& other) const {
                    return !(*this != other);
                }

                TIteratorState Iterators_;
                THolders* HoldersPtr_;
            };
        public:
            using iterator = TIterator;
            using const_iterator = TIterator;

            TIterator begin() const {
                bool isEmpty = !((std::begin(*std::get<I>(Holders_).Ptr()) != std::end(*std::get<I>(Holders_).Ptr())) && ...);
                return {TIteratorState{int(isEmpty), std::begin(*std::get<I>(Holders_).Ptr())...}, &Holders_};
            }

            TSentinel end() const {
                return {TSentinelState{1, std::end(*std::get<I>(Holders_).Ptr())...}, &Holders_};
            }

            mutable THolders Holders_;
        };

        template <std::size_t... I>
        static auto CartesianMultiply(TContainers&&... containers, std::index_sequence<I...>) {
            return TCartesianMultiplierWithIndex<I...>{{std::forward<TContainers>(containers)...}};
        }
    };


    template <typename TValue_, typename... TContainers>
    struct TConcatenator {
        template <std::size_t... I>
        struct TConcatenatorWithIndex {
        private:
            using THolders = std::tuple<TAutoEmbedOrPtrPolicy<TContainers>...>;
            using TValue = TValue_;
            using TIteratorState = std::tuple<decltype(std::begin(std::declval<TContainers&>()))...>;
            using TSentinelState = std::tuple<decltype(std::end(std::declval<TContainers&>()))...>;

            struct TIterator;
            struct TSentinelCandidate {
                TSentinelState Iterators_;
                std::size_t Position_;
                THolders* HoldersPtr_;
            };
            using TSentinel = std::conditional_t<std::is_same_v<TIteratorState, TSentinelState>,
                                                 TIterator, TSentinelCandidate>;

            struct TIterator {
            private:
                friend struct TConcatenatorWithIndex<I...>;

                // important, that it is a static function, compiler better optimizes such code
                template <std::size_t index = 0>
                static TValue GetCurrentValue(std::size_t position, TIteratorState& iterators) {
                    if constexpr (index >= sizeof...(TContainers)) {
                        // never happened when use of iterator is correct
                        return *std::get<0>(iterators);
                    } else {
                        if (position == index) {
                            return *std::get<index>(iterators);
                        } else {
                            return GetCurrentValue<index + 1>(position, iterators);
                        }
                    }
                }

                template <bool needIncrement, std::size_t index = 0>
                void MaybeIncrementIteratorAndSkipExhaustedContainers() {
                    if constexpr (index >= sizeof...(TContainers)) {
                        return;
                    } else {
                        if (Position_ == index) {
                            if constexpr (needIncrement) {
                                ++std::get<index>(Iterators_);
                            }
                            if (!(std::get<index>(Iterators_) != std::end(*std::get<index>(*HoldersPtr_).Ptr()))) {
                                ++Position_;
                                MaybeIncrementIteratorAndSkipExhaustedContainers<false, index + 1>();
                            }
                        } else {
                            MaybeIncrementIteratorAndSkipExhaustedContainers<needIncrement, index + 1>();
                        }
                    }
                }
            public:
                using difference_type = std::ptrdiff_t;
                using value_type = TValue;
                using pointer = std::remove_reference_t<TValue>*;
                using reference = std::remove_reference_t<TValue>&;
                using iterator_category = std::input_iterator_tag;

                TValue operator*() {
                    return GetCurrentValue(Position_, Iterators_);
                }
                void operator++() {
                    MaybeIncrementIteratorAndSkipExhaustedContainers<true>();
                }
                bool operator!=(const TSentinel& other) const {
                    // give compiler an opportunity to optimize sentinel case (-70% of time)
                    if (other.Position_ == sizeof...(TContainers)) {
                        return Position_ < sizeof...(TContainers);
                    } else {
                        return (Position_ != other.Position_ ||
                                ((std::get<I>(Iterators_) != std::get<I>(other.Iterators_)) || ...));
                    }
                }
                bool operator==(const TSentinel& other) const {
                    return !(*this != other);
                }

                TIteratorState Iterators_;
                std::size_t Position_;
                THolders* HoldersPtr_;
            };
        public:
            using iterator = TIterator;
            using const_iterator = TIterator;

            TIterator begin() const {
                TIterator iterator{TIteratorState{std::begin(*std::get<I>(Holders_).Ptr())...}, 0, &Holders_};
                iterator.template MaybeIncrementIteratorAndSkipExhaustedContainers<false>();
                return iterator;
            }

            TSentinel end() const {
                return {TSentinelState{std::end(*std::get<I>(Holders_).Ptr())...}, sizeof...(TContainers), &Holders_};
            }

            mutable THolders Holders_;
        };

        template <std::size_t... I>
        static auto Concatenate(TContainers&&... containers, std::index_sequence<I...>) {
            return TConcatenatorWithIndex<I...>{{std::forward<TContainers>(containers)...}};
        }
    };


    struct TTupleRecursiveFlattener {

        template <class TObject, std::size_t... I>
        static auto FlattenTuple(TObject&& object, std::index_sequence<I...>) {
            return std::tuple_cat(Flatten(std::forward<std::tuple_element_t<I, std::decay_t<TObject>>>(std::get<I>(object)), 0u)...);
        }

        template <class TObject, size_t Size = std::tuple_size<std::decay_t<TObject>>::value>
        static auto Flatten(TObject&& object, uint32_t) {
            auto indexes = std::make_index_sequence<Size>{};
            return FlattenTuple(std::forward<TObject>(object), indexes);
        }

        template <class TObject>
        static auto Flatten(TObject&& object, char) {
            return std::tuple<TObject>(std::forward<TObject>(object));
        }

        template <class TObject>
        auto operator()(TObject&& object) const {
            return Flatten(std::forward<TObject>(object), 0u);
        }

    };
}

namespace NFuncTools {
    //! Usage: for (auto [i, x] : Enumerate(container)) {...}
    template <typename TContainerOrRef>
    auto Enumerate(TContainerOrRef&& container) {
        return NFuncToolsPrivate::TEnumerator<TContainerOrRef>{std::forward<TContainerOrRef>(container)};
    }

    using ::Reversed;

    template <typename TValue>
    auto Range(TValue from, TValue to, TValue step) {
        return xrange(from, to, step);
    }

    template <typename TValue>
    auto Range(TValue from, TValue to) {
        return xrange(from, to);
    }

    template <typename TValue>
    auto Range(TValue to) {
        return xrange(to);
    }

    //! Acts as pythonic zip, BUT result length is equal to shortest lenght of input containers
    //! Usage: for (auto [ai, bi, ci] : Zip(a, b, c)) {...}

    template <typename... TContainers>
    auto Zip(TContainers&&... containers) {
        return NFuncToolsPrivate::TZipper<TContainers...>::Zip(
            std::forward<TContainers>(containers)...,
            std::make_index_sequence<sizeof...(TContainers)>{}
        );
    }

    //! Usage: for (i32 x : Map([](i32 x) { return x * x; }, a)) {...}
    template <typename TMapper, typename TContainerOrRef>
    auto Map(TMapper&& mapper, TContainerOrRef&& container) {
        return ::MakeMappedRange(std::forward<TContainerOrRef>(container), std::forward<TMapper>(mapper));
    }

    //! Usage: for (auto i : Map<int>(floats)) {...}
    template <typename TMapResult, typename TContainerOrRef>
    auto Map(TContainerOrRef&& container) {
        return Map([](const auto& x) { return TMapResult(x); }, std::forward<TContainerOrRef>(container));
    }

    //! Usage: for (i32 x : Filter(predicate, container)) {...}
    template <typename TPredicate, typename TContainerOrRef>
    auto Filter(TPredicate&& predicate, TContainerOrRef&& container) {
        return ::MakeFilteringRange(std::forward<TContainerOrRef>(container), std::forward<TPredicate>(predicate));
    }

    //! Usage: for (auto [ai, bi] : CartesianProduct(a, b)) {...}
    //! Equivalent: for (auto& ai : a) { for (auto& bi : b) {...} }
    template <typename... TContainers>
    auto CartesianProduct(TContainers&&... containers) {
        return NFuncToolsPrivate::TCartesianMultiplier<TContainers...>::CartesianMultiply(std::forward<TContainers>(containers)..., std::make_index_sequence<sizeof...(TContainers)>{});
    }

    //! Usage: for (auto x : Concatenate(a, b)) {...}
    template <typename TFirstContainer, typename... TContainers>
    auto Concatenate(TFirstContainer&& container, TContainers&&... containers) {
        return NFuncToolsPrivate::TConcatenator<decltype(*std::begin(container)), TFirstContainer, TContainers...>::Concatenate(
            std::forward<TFirstContainer>(container), std::forward<TContainers>(containers)..., std::make_index_sequence<sizeof...(TContainers) + 1>{});
    }

    //! Usage: for (auto [i, ai, bi] : Flatten(Enumerate(Zip(a, b))) {...}
    template <typename TContainerOrRef>
    auto Flatten(TContainerOrRef&& container) {
        return Map(NFuncToolsPrivate::TTupleRecursiveFlattener{}, std::forward<TContainerOrRef>(container));
    }

}
