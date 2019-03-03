#pragma once

#include <iterator>
#include <iterator_range.h>

template <class TIterator, class TCondition>
class TFilteringIterator {
public:
    using TSelf = TFilteringIterator<TIterator, TCondition>;

    using difference_type = typename std::iterator_traits<TIterator>::difference_type;
    using value_type = typename std::iterator_traits<TIterator>::value_type;
    using reference = typename std::iterator_traits<TIterator>::reference;
    using pointer = typename std::iterator_traits<TIterator>::pointer;
    using iterator_category = std::forward_iterator_tag;

    TFilteringIterator(TIterator it, TIterator last, const TCondition& condition)
        : Iter(it)
        , Last(last)
        , Condition(condition)
    {
        Grep();
    }

    TSelf& operator=(const TSelf&) = default;

    TSelf& operator++() {
        ++Iter;
        Grep();
        return *this;
    }

    decltype(auto) operator*() const {
        return *Iter;
    }

    pointer operator->() const {
        return &*Iter;
    }

    bool operator==(const TSelf& other) const {
        return Iter == other.Iter;
    }
    bool operator!=(const TSelf& other) const {
        return Iter != other.Iter;
    }

private:
    void Grep() {
        while (Iter != Last && !Condition(*Iter)) {
            ++Iter;
        }
    }
    TIterator Iter;
    TIterator Last;
    TCondition Condition;
};


template <class TContainer, class TCondition>
class TFilteringRange {
    using TContainerStorage = TAutoEmbedOrPtrPolicy<TContainer>;
    using TConditionStorage = TAutoEmbedOrPtrPolicy<TCondition>;
    using TRawIterator = decltype(std::begin(std::declval<TContainer&>()));

    struct TConditionWrapper {
        template <class T>
        bool operator()(T&& x) const {
            return (*Condition)(std::forward<T>(x));
        }
        std::remove_reference_t<TCondition>* Condition;
    };
public:
    using TIterator = TFilteringIterator<TRawIterator, TConditionWrapper>;
    using iterator = TIterator;
    using const_iterator = TIterator;

    TFilteringRange(TContainer&& container, TCondition&& predicate)
        : Container(std::forward<TContainer>(container))
        , Condition(std::forward<TCondition>(predicate))
    {}

    TIterator begin() {
        return {std::begin(*Container.Ptr()), std::end(*Container.Ptr()), {Condition.Ptr()}};
    }

    TIterator end() {
        return {std::end(*Container.Ptr()), std::end(*Container.Ptr()), {Condition.Ptr()}};
    }

private:
    TContainerStorage Container;
    TConditionStorage Condition;
};


template <class TIterator, class TCondition>
auto MakeFilteringRange(TIterator begin, TIterator end, const TCondition& condition) {
    return MakeIteratorRange(TFilteringIterator<TIterator, TCondition>(begin, end, condition), TFilteringIterator<TIterator, TCondition>(end, end, condition));
}

template <class TContainer, class TCondition>
auto MakeFilteringRange(TContainer&& container, TCondition&& condition) {
    return TFilteringRange<TContainer, TCondition>(std::forward<TContainer>(container), std::forward<TCondition>(condition));
}
