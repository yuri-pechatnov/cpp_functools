
#pragma once

#include <range/v3/view/concat.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/cartesian_product.hpp>

#include <functools_helpers.h>

#include <vector>


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
    }

    // range-v3 can't handle lvalues :(
    template <typename TFirstContainer, typename... TContainers>
    auto Concatenate(TFirstContainer& container, TContainers&... containers) {
        return ranges::view::concat(container, containers...);
    }

}
