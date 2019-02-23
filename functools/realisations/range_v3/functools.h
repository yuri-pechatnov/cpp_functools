
#pragma once

#include <range/v3/view/concat.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/cartesian_product.hpp>

#include <functools_helpers.h>

#include <vector>

namespace NFuncTools::NPrivate {
    template <typename... TContainers>
    struct TZipper {
        template <std::size_t... I>
        static auto Zip(TContainers&&... containers, std::index_sequence<I...>) {
            using namespace NHelpers;

            return TGeneratorRange(
                [holders = std::tuple{TAutoEmbedOrPtrPolicy<TContainers>(containers)...}]() mutable {
                    auto iters = std::tuple{std::get<I>(holders).Ptr()->begin()...};
                    return TGeneratorRangeIterator(
                        [&holders, iters](auto query) mutable {
                            if constexpr (query.GetCurrent) {
                                // std::forward_as_tuple is bad idea, because we should not take references to objects that are not returned by reference
                                return std::tuple<decltype(*std::declval<TContainers>().begin())...>(*std::get<I>(iters)...);
                            }
                            if constexpr (query.IsNotEnd) {
                                return ((std::get<I>(iters) != std::get<I>(holders).Ptr()->end()) && ...);
                            }
                            if constexpr (query.Next) {
                                (++std::get<I>(iters), ...);
                            }
                        });
                });
        }
    };

    template <typename... TContainers>
    struct TCartesianMultiplier {
        //! Return value is true when iteration is finished
        template <typename TIteratorsTuple, typename THoldersTuple,
                  std::size_t position = std::tuple_size<TIteratorsTuple>::value - 1>
        static bool IncrementIteratorsTuple(TIteratorsTuple& iteratorsTuple, THoldersTuple& holdersTuple) {
            auto& currentIterator = std::get<position>(iteratorsTuple);
            ++currentIterator.Get();

            if (!(currentIterator.Get() != std::get<position>(holdersTuple).Ptr()->end())) {
                currentIterator = std::get<position>(holdersTuple).Ptr()->begin();
                if constexpr (position == 0) {
                    return true;
                } else {
                    return IncrementIteratorsTuple<TIteratorsTuple, THoldersTuple, position - 1>(iteratorsTuple, holdersTuple);
                }
            } else {
                return false;
            }
        }

        template <std::size_t... I>
        static auto CartesianMultiply(TContainers&&... containers, std::index_sequence<I...>) {
            using namespace NHelpers;

            return TGeneratorRange(
                [holders = std::tuple{TAutoEmbedOrPtrPolicy<TContainers>(containers)...}]() mutable {
                    auto iters = std::tuple{TCopyAssignableHolder(std::get<I>(holders).Ptr()->begin())...};
                    bool finished = !((std::get<I>(iters).Get() != std::get<I>(holders).Ptr()->end()) && ...);

                    return TGeneratorRangeIterator(
                        [&holders, iters, finished](auto query) mutable -> decltype(auto) {
                            if constexpr (query.GetCurrent) {
                                return std::tuple<decltype(*std::declval<TContainers>().begin())...>(*std::get<I>(iters).Get()...);
                            }
                            if constexpr (query.IsNotEnd) {
                                return !finished;
                            }
                            if constexpr (query.Next) {
                                finished |= IncrementIteratorsTuple(iters, holders);
                            }
                        });
                });
        }
    };

    template <typename TValue, typename... TContainers>
    struct TConcatenator {
        template <std::size_t index = 0, typename TIters>
        static TValue GetCurrentValue(std::size_t position, TIters& iters) {
            if constexpr (index >= sizeof...(TContainers)) {
                // never happened when use of iterator is correct
                return *std::get<0>(iters);
            } else {
                if (position == index) {
                    return *std::get<index>(iters);
                } else {
                    return GetCurrentValue<index + 1>(position, iters);
                }
            }
        }

        template <bool needIncrement, std::size_t index = 0, typename TIters, typename THolders>
        static void MaybeIncrementIteratorAndSkipExhaustedContainers(std::size_t& position, TIters& iters, THolders& holders) {
            if constexpr (index >= sizeof...(TContainers)) {
                return;
            } else {
                if (position == index) {
                    if constexpr (needIncrement) {
                        ++std::get<index>(iters);
                    }
                    if (std::get<index>(iters) == std::get<index>(holders).Ptr()->end()) {
                        ++position;
                        MaybeIncrementIteratorAndSkipExhaustedContainers<false, index + 1>(position, iters, holders);
                    }
                } else {
                    MaybeIncrementIteratorAndSkipExhaustedContainers<needIncrement, index + 1>(position, iters, holders);
                }
            }
        }

        template <std::size_t... I>
        static auto Concatenate(TContainers&&... containers, std::index_sequence<I...>) {
            using namespace NHelpers;

             return TGeneratorRange(
                [holders = std::tuple{TAutoEmbedOrPtrPolicy<TContainers>(containers)...}]() mutable {
                    auto iters = std::tuple{std::get<I>(holders).Ptr()->begin()...};

                    std::size_t position = 0;
                    MaybeIncrementIteratorAndSkipExhaustedContainers<false>(position, iters, holders);

                    return TGeneratorRangeIterator(
                        [&holders, iters, position](auto query) mutable -> decltype(auto) {
                            if constexpr (query.GetCurrent) {
                                return GetCurrentValue(position, iters);
                            }
                            if constexpr (query.IsNotEnd) {
                                return position < sizeof...(TContainers);
                            }
                            if constexpr (query.Next) {
                                MaybeIncrementIteratorAndSkipExhaustedContainers<true>(position, iters, holders);
                            }
                        });
                });
        }
    };

    struct TTupleRecursiveFlattener {

        template <class TObject, std::size_t... I>
        auto FlattenTuple(TObject&& object, std::index_sequence<I...>) {
            return std::tuple_cat(Flatten(std::get<I>(std::forward<TObject>(object)), 0u)...);
        }

        template <class TObject,
                  size_t Size = std::tuple_size<typename std::decay<TObject>::type>::value>
        auto Flatten(TObject&& object, uint32_t) {
            auto indexes = std::make_index_sequence<Size>{};
            return FlattenTuple(std::forward<TObject>(object), indexes);
        }

        template <class TObject>
        auto Flatten(TObject&& object, char) {
            return std::tuple<TObject>(std::forward<TObject>(object));
        }

        template <class TObject>
        auto operator()(TObject&& object) {
            return Flatten(std::forward<TObject>(object), 0u);
        }

    };
}

namespace NFuncTools {
    template <typename TContainerOrRef>
    auto Enumerate(TContainerOrRef&& container) {
        return ranges::view::enumerate(std::forward<TContainerOrRef>(container));
    }

    template <typename TContainerOrRef>
    auto Reversed(TContainerOrRef&& container) {
        return ranges::view::reverse(std::forward<TContainerOrRef>(container));
    }

    template <typename TValue>
    auto Range(TValue from, TValue to, TValue step) {
        return ranges::view::iota(from, to, step);
    }

    template <typename TValue>
    auto Range(TValue from, TValue to) {
        return ranges::view::iota(from, to);
    }

    template <typename TValue>
    auto Range(TValue to) {
        return ranges::view::iota(0, to);
    }

    //! Acts as pythonic zip, BUT result length is equal to shortest lenght of input containers
    //! Usage: for (auto [ai, bi, ci] : Zip(a, b, c)) {...}
    template <typename... TContainers>
    auto Zip(TContainers&&... containers) {
        return ranges::view::zip(std::forward<TContainers>(containers)...);
    }

    //! Usage: for (i32 x : Map([](i32 x) { return x * x; }, a)) {...}
    template <typename TMapper, typename TContainerOrRef>
    auto Map(TMapper&& mapper, TContainerOrRef&& container) {
        return ranges::view::transform(std::forward<TContainerOrRef>(container), std::forward<TMapper>(mapper));
    }

    //! Usage: for (auto i : Map<int>(floats)) {...}
    template <typename TMapResult, typename TContainerOrRef>
    auto Map(TContainerOrRef&& container) {
        return Map([](const auto& x) { return TMapResult(x); }, std::forward<TContainerOrRef>(container));
    }

    //! Usage: for (i32 x : Filter(predicate, container)) {...}
    template <typename TPredicate, typename TContainerOrRef>
    auto Filter(TPredicate&& predicate, TContainerOrRef&& container) {
        return ranges::view::filter(std::forward<TContainerOrRef>(container), std::forward<TPredicate>(predicate));
    }

    //! Usage: for (auto [ai, bi] : CartesianProduct(a, b)) {...}
    //! Equivalent: for (auto& ai : a) { for (auto& bi : b) {...} }
    template <typename... TContainers>
    auto CartesianProduct(TContainers&&... containers) {

        return ranges::view::cartesian_product(std::forward<TContainers>(containers)...);
        //~ return NPrivate::TCartesianMultiplier<TContainers...>::CartesianMultiply(std::forward<TContainers>(containers)..., std::make_index_sequence<sizeof...(TContainers)>{});
    }

    // range-v3 can't handle lvalues :(
    template <typename TFirstContainer, typename... TContainers>
    auto Concatenate(TFirstContainer& container, TContainers&... containers) {
        return ranges::view::concat(container, containers...);
    }

    //! Usage: for (auto [i, ai, bi] : Flatten(Enumerate(Zip(a, b))) {...}
    template <typename TContainerOrRef>
    auto Flatten(TContainerOrRef&& container) {
        return Map(NPrivate::TTupleRecursiveFlattener{}, std::forward<TContainerOrRef>(container));
    }

}
