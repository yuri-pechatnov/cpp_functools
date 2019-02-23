#pragma once

#include <tc/zip_range.h>
#include <tc/reverse_adaptor.h>
#include <tc/filter_adaptor.h>
#include <tc/concat_adaptor.h>
#include <tc/transform.h>


namespace NFuncTools {

    template <typename TContainerOrRef>
    auto Reversed(TContainerOrRef&& container) {
        return tc::reverse(std::forward<TContainerOrRef>(container));
    }

    //! Acts as pythonic zip, BUT result length is equal to shortest lenght of input containers
    //! Usage: for (auto [ai, bi, ci] : Zip(a, b, c)) {...}
    template <typename... TContainers>
    auto Zip(TContainers&&... containers) {
        return tc::zip(std::forward<TContainers>(containers)...);
    }

    //! Usage: for (i32 x : Map([](i32 x) { return x * x; }, a)) {...}
    template <typename TMapper, typename TContainerOrRef>
    auto Map(TMapper&& mapper, TContainerOrRef&& container) {
        return tc::transform(std::forward<TContainerOrRef>(container), std::forward<TMapper>(mapper));
    }

    //! Usage: for (auto i : Map<int>(floats)) {...}
    template <typename TMapResult, typename TContainerOrRef>
    auto Map(TContainerOrRef&& container) {
        return Map([](const auto& x) { return TMapResult(x); }, std::forward<TContainerOrRef>(container));
    }

    //! Usage: for (i32 x : Filter(predicate, container)) {...}
    template <typename TPredicate, typename TContainerOrRef>
    auto Filter(TPredicate&& predicate, TContainerOrRef&& container) {
        return tc::filter(std::forward<TContainerOrRef>(container), std::forward<TPredicate>(predicate));
    }


    // range-v3 can't handle lvalues :(
    template <typename TFirstContainer, typename... TContainers>
    auto Concatenate(TFirstContainer&& container, TContainers&&... containers) {
        return tc::concat(std::forward<TFirstContainer>(container), std::forward<TContainers>(containers)...);
    }

}
