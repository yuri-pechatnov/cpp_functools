#pragma once

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/combine.hpp>

#include <vector>

namespace NFuncTools {

    template <typename TContainerOrRef>
    auto Reversed(TContainerOrRef&& container) {
        return boost::adaptors::reverse(container);
    }

    template <typename TValue>
    auto Range(TValue from, TValue to, TValue step) {
        return boost::irange(from, to, step);
    }

    template <typename TValue>
    auto Range(TValue from, TValue to) {
        return boost::irange(from, to);
    }

    template <typename TValue>
    auto Range(TValue to) {
        return boost::irange(to);
    }

    //! Acts as pythonic zip, BUT result length is equal to shortest lenght of input containers
    //! Usage: for (auto [ai, bi, ci] : Zip(a, b, c)) {...}
    template <typename... TContainers>
    auto Zip(TContainers&&... containers) {
        return boost::combine(std::forward<TContainers>(containers)...);
    }

    //! Usage: for (i32 x : Map([](i32 x) { return x * x; }, a)) {...}
    template <typename TMapper, typename TContainerOrRef>
    auto Map(TMapper&& mapper, TContainerOrRef&& container) {
        return boost::adaptors::transform(container, mapper);
    }

    //! Usage: for (auto i : Map<int>(floats)) {...}
    template <typename TMapResult, typename TContainerOrRef>
    auto Map(TContainerOrRef&& container) {
        return Map([](const auto& x) { return TMapResult(x); }, std::forward<TContainerOrRef>(container));
    }

    //! Usage: for (i32 x : Filter(predicate, container)) {...}
    template <typename TPredicate, typename TContainerOrRef>
    auto Filter(TPredicate&& predicate, TContainerOrRef&& container) {
        return boost::adaptors::filter(container, predicate);
    }

}
