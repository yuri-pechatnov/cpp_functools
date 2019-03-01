#pragma once

#include <reverse.h>
#include <xrange.h>
#include <mapped.h>
#include <filtering.h>

#include <tuple>
#include <vector>

namespace NFuncTools::NPrivate {
    template <typename... TContainers>
    struct TZipper {
        template <std::size_t... I>
        struct TZipperWithIndex {
            using THolders = std::tuple<TAutoEmbedOrPtrPolicy<TContainers>...>;
            using TValue = std::tuple<decltype(*std::declval<TContainers>().begin())...>;
            using TIteratorState = std::tuple<decltype(std::declval<TContainers>().begin())...>;
            using TSentinelState = std::tuple<decltype(std::declval<TContainers>().end())...>;

            struct TIterator;
            struct TSentinelCandidate {
                TSentinelState Iterators;
            };
            using TSentinel = typename std::conditional<
                std::is_same<TIteratorState, TSentinelState>::value, TIterator, TSentinelCandidate>::type;

            struct TIterator {
                using difference_type = std::ptrdiff_t;
                using value_type = TValue;
                using pointer = TValue*;
                using reference = TValue&;
                using iterator_category = std::input_iterator_tag;

                TValue operator*() {
                    return {*std::get<I>(Iterators)...};
                }
                void operator++() {
                    (++std::get<I>(Iterators), ...);
                }
                bool operator!=(const TSentinel& other) const {
                    // yes, for all correct iterators but end() it is a correct way to compare
                    return ((std::get<I>(Iterators) != std::get<I>(other.Iterators)) && ...);
                }
                bool operator==(const TSentinel& other) const {
                    return !(*this != other);
                }

                TIteratorState Iterators;
            };

            using iterator = TIterator;

            TIterator begin() {
                return {TIteratorState{std::get<I>(Holders).Ptr()->begin()...}};
            }

            TSentinel end() {
                return {TSentinelState{std::get<I>(Holders).Ptr()->end()...}};
            }

            THolders Holders;
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
            using THolders = std::tuple<TAutoEmbedOrPtrPolicy<TContainers>...>;
            using TValue = std::tuple<decltype(*std::declval<TContainers>().begin())...>;
            using TIteratorState = std::tuple<int, decltype(std::declval<TContainers>().begin())...>;
            using TSentinelState = std::tuple<int, decltype(std::declval<TContainers>().end())...>;

            struct TIterator;
            struct TSentinelCandidate {
                TSentinelState Iterators;
                THolders* HoldersPtr;
            };
            using TSentinel = typename std::conditional<
                std::is_same<TIteratorState, TSentinelState>::value, TIterator, TSentinelCandidate>::type;

            struct TIterator {
                using difference_type = std::ptrdiff_t;
                using value_type = TValue;
                using pointer = TValue*;
                using reference = TValue&;
                using iterator_category = std::input_iterator_tag;

                //! Return value is true when iteration is not finished
                template <std::size_t position = std::tuple_size<TIteratorState>::value - 1>
                void IncrementIteratorsTuple() {
                    auto& currentIterator = std::get<position>(Iterators);
                    ++currentIterator;

                    if constexpr (position != 0) {
                        if (currentIterator != std::get<position - 1>(*HoldersPtr).Ptr()->end()) {
                            return; // most frequent case
                        } else {
                            currentIterator = std::get<position - 1>(*HoldersPtr).Ptr()->begin();
                            IncrementIteratorsTuple<position - 1>();
                        }
                    }
                }

                TValue operator*() {
                    return {*std::get<I + 1>(Iterators)...};
                }
                void operator++() {
                    IncrementIteratorsTuple();
                }
                bool operator!=(const TSentinel& other) const {
                    // not finished iterator VS sentinel (most frequent case)
                    if (std::get<0>(Iterators) != std::get<0>(other.Iterators)) {
                        return true;
                    }
                    // do not compare sentinels and finished iterators
                    if (std::get<0>(other.Iterators)) {
                        return false;
                    }
                    // compare not finished iterators
                    return ((std::get<I + 1>(Iterators) != std::get<I + 1>(other.Iterators)) || ...);
                }
                bool operator==(const TSentinel& other) const {
                    return !(*this != other);
                }

                TIteratorState Iterators;
                THolders* HoldersPtr;
            };

            using iterator = TIterator;

            TIterator begin() {
                bool isEmpty = !((std::get<I>(Holders).Ptr()->begin() != std::get<I>(Holders).Ptr()->end()) && ...);
                return {TIteratorState{int(isEmpty), std::get<I>(Holders).Ptr()->begin()...}, &Holders};
            }

            TSentinel end() {
                return {TSentinelState{1, std::get<I>(Holders).Ptr()->end()...}, &Holders};
            }

            THolders Holders;
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
            using THolders = std::tuple<TAutoEmbedOrPtrPolicy<TContainers>...>;
            using TValue = TValue_;
            using TIteratorState = std::tuple<decltype(std::declval<TContainers>().begin())...>;
            using TSentinelState = std::tuple<decltype(std::declval<TContainers>().end())...>;

            struct TIterator;
            struct TSentinelCandidate {
                TSentinelState Iterators;
                std::size_t Position;
                THolders* HoldersPtr;
            };
            using TSentinel = typename std::conditional<
                std::is_same<TIteratorState, TSentinelState>::value, TIterator, TSentinelCandidate>::type;

            struct TIterator {
                using difference_type = std::ptrdiff_t;
                using value_type = TValue;
                using pointer = std::remove_reference_t<TValue>*;
                using reference = std::remove_reference_t<TValue>&;
                using iterator_category = std::input_iterator_tag;

                template <std::size_t index = 0>
                TValue GetCurrentValue() {
                    if constexpr (index >= sizeof...(TContainers)) {
                        // never happened when use of iterator is correct
                        return *std::get<0>(Iterators);
                    } else {
                        if (Position == index) {
                            return *std::get<index>(Iterators);
                        } else {
                            return GetCurrentValue<index + 1>();
                        }
                    }
                }

                template <bool needIncrement, std::size_t index = 0>
                void MaybeIncrementIteratorAndSkipExhaustedContainers() {
                    if constexpr (index >= sizeof...(TContainers)) {
                        return;
                    } else {
                        if (Position == index) {
                            if constexpr (needIncrement) {
                                ++std::get<index>(Iterators);
                            }
                            if (std::get<index>(Iterators) == std::get<index>(*HoldersPtr).Ptr()->end()) {
                                ++Position;
                                MaybeIncrementIteratorAndSkipExhaustedContainers<false, index + 1>();
                            }
                        } else {
                            MaybeIncrementIteratorAndSkipExhaustedContainers<needIncrement, index + 1>();
                        }
                    }
                }


                TValue operator*() {
                    return GetCurrentValue();
                }
                void operator++() {
                    MaybeIncrementIteratorAndSkipExhaustedContainers<true>();
                }
                bool operator!=(const TSentinel& other) const {
                    // give compiler an opportunity to optimize sentinel case
                    if (other.Position == sizeof...(TContainers)) {
                        return (Position != other.Position);
                    } else {
                        return (Position != other.Position ||
                                ((std::get<I>(Iterators) != std::get<I>(other.Iterators)) || ...));
                    }
                }
                bool operator==(const TSentinel& other) const {
                    return !(*this != other);
                }

                TIteratorState Iterators;
                std::size_t Position;
                THolders* HoldersPtr;
            };

            using iterator = TIterator;

            TIterator begin() {
                TIterator iterator{TIteratorState{std::get<I>(Holders).Ptr()->begin()...}, 0, &Holders};
                iterator.template MaybeIncrementIteratorAndSkipExhaustedContainers<false>();
                return iterator;
            }

            TSentinel end() {
                return {TSentinelState{std::get<I>(Holders).Ptr()->end()...}, sizeof...(TContainers), &Holders};
            }

            THolders Holders;
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

        template <class TObject,
                  size_t Size = std::tuple_size<typename std::decay<TObject>::type>::value>
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


    template <typename TContainer>
    struct TEnumerator {
        using TStorage = TAutoEmbedOrPtrPolicy<TContainer>;
        using TValue = std::tuple<const std::size_t&, decltype(*std::declval<TContainer>().begin())>;
        using TIteratorState = decltype(std::declval<TContainer>().begin());
        using TSentinelState = decltype(std::declval<TContainer>().end());

        struct TIterator;
        struct TSentinelCandidate {
            std::size_t Index;
            TSentinelState Iterator;
        };
        using TSentinel = typename std::conditional<
            std::is_same<TIteratorState, TSentinelState>::value, TIterator, TSentinelCandidate>::type;

        struct TIterator {
            using difference_type = std::ptrdiff_t;
            using value_type = TValue;
            using pointer = TValue*;
            using reference = TValue&;
            using iterator_category = std::input_iterator_tag;

            TValue operator*() {
                return {Index, *Iterator};
            }
            TValue operator*() const {
                return {Index, *Iterator};
            }
            void operator++() {
                ++Index;
                ++Iterator;
            }
            bool operator!=(const TSentinel& other) const {
                return Iterator != other.Iterator;
            }
            bool operator==(const TSentinel& other) const {
                return Iterator == other.Iterator;
            }

            std::size_t Index;
            TIteratorState Iterator;
        };

        using iterator = TIterator;

        TIterator begin() {
            return {0, Storage.Ptr()->begin()};
        }

        TSentinel end() {
            return {~0u, Storage.Ptr()->end()};
        }

        TStorage Storage;
    };
}

namespace NFuncTools {
    //! Usage: for (auto [i, x] : Enumerate(container)) {...}
    template <typename TContainerOrRef>
    auto Enumerate(TContainerOrRef&& container) {
        return NPrivate::TEnumerator<TContainerOrRef>{std::forward<TContainerOrRef>(container)};
    }

    template <typename TContainerOrRef>
    auto Reversed(TContainerOrRef&& container) {
        return ::Reversed(std::forward<TContainerOrRef>(container));
    }

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
        return NPrivate::TZipper<TContainers...>::Zip(
            std::forward<TContainers>(containers)...,
            std::make_index_sequence<sizeof...(TContainers)>{}
        );
    }

    //! Usage: for (i32 x : Map([](i32 x) { return x * x; }, a)) {...}
    template <typename TMapper, typename TContainerOrRef>
    auto Map(TMapper&& mapper, TContainerOrRef&& container) {
        return ::MakeSimpleMappedRange(std::forward<TContainerOrRef>(container), std::forward<TMapper>(mapper));
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
        return NPrivate::TCartesianMultiplier<TContainers...>::CartesianMultiply(std::forward<TContainers>(containers)..., std::make_index_sequence<sizeof...(TContainers)>{});
    }

    //! Usage: for (auto x : Concatenate(a, b)) {...}
    template <typename TFirstContainer, typename... TContainers>
    auto Concatenate(TFirstContainer&& container, TContainers&&... containers) {
        return NPrivate::TConcatenator<decltype(*container.begin()), TFirstContainer, TContainers...>::Concatenate(
            std::forward<TFirstContainer>(container), std::forward<TContainers>(containers)..., std::make_index_sequence<sizeof...(TContainers) + 1>{});
    }

    //! Usage: for (auto [i, ai, bi] : Flatten(Enumerate(Zip(a, b))) {...}
    template <typename TContainerOrRef>
    auto Flatten(TContainerOrRef&& container) {
        return Map(NPrivate::TTupleRecursiveFlattener{}, std::forward<TContainerOrRef>(container));
    }

}
