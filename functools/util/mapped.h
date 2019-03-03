#pragma once

#include <iterator>
#include <iterator_range.h>
#include <store_policy.h>

template <class TIterator, class TMapper>
class TMappedIterator {
public:
    using TSelf = TMappedIterator<TIterator, TMapper>;

    using TSrcPointerType = typename std::iterator_traits<TIterator>::reference;
    using TValue = decltype(std::declval<TMapper>()(std::declval<TSrcPointerType>()));

    using difference_type = std::ptrdiff_t;
    using value_type = TValue;
    using reference = TValue&;
    using pointer = std::remove_reference_t<TValue>*;
    using iterator_category = std::input_iterator_tag;

    TMappedIterator(TIterator it, TMapper mapper)
        : Iter(it)
        , Mapper(mapper)
    {
    }

    TSelf& operator++() {
        ++Iter;
        return *this;
    }
    TSelf& operator--() {
        --Iter;
        return *this;
    }
    TValue operator*() {
        return Mapper((*Iter));
    }
    TValue operator*() const {
        return Mapper((*Iter));
    }

    pointer operator->() const {
        return &(Mapper((*Iter)));
    }

    TValue operator[](difference_type n) const {
        return Mapper(*(Iter + n));
    }
    TSelf& operator+=(difference_type n) {
        Iter += n;
        return *this;
    }
    TSelf& operator-=(difference_type n) {
        Iter -= n;
        return *this;
    }
    TSelf operator+(difference_type n) const {
        return TSelf(Iter + n, Mapper);
    }
    difference_type operator-(const TSelf& other) const {
        return Iter - other.Iter;
    }
    bool operator==(const TSelf& other) const {
        return Iter == other.Iter;
    }
    bool operator!=(const TSelf& other) const {
        return Iter != other.Iter;
    }
    bool operator>(const TSelf& other) const {
        return Iter > other.Iter;
    }
    bool operator<(const TSelf& other) const {
        return Iter < other.Iter;
    }

private:
    TIterator Iter;
    TMapper Mapper;
};


//! Doesn't support const iterators and some methods, so it has lower requirements to TContainer
template <class TContainer, class TMapper>
class TSimpleOwningMappedIteratorRange {
public:
    using TContainerStorage = TAutoEmbedOrPtrPolicy<TContainer>;
    using TMapperStorage = TAutoEmbedOrPtrPolicy<TMapper>;
    using InternalIterator = decltype(std::begin(std::declval<TContainer&>()));

    struct TMapperWrapper {
        template <class T>
        decltype(auto) operator()(T&& x) const {
            return (*Mapper)(std::forward<T>(x));
        }
        std::remove_reference_t<TMapper>* Mapper;
    };

    using Iterator = TMappedIterator<InternalIterator, TMapperWrapper>;
    using iterator = Iterator;
    using value_type = typename std::iterator_traits<iterator>::value_type;
    using reference = typename std::iterator_traits<iterator>::reference;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;

    TSimpleOwningMappedIteratorRange(TContainer&& container, TMapper&& mapper)
        : Container(std::forward<TContainer>(container))
        , Mapper(std::forward<TMapper>(mapper))
    {
    }

    Iterator begin() {
        return {std::begin(*Container.Ptr()), {Mapper.Ptr()}};
    }

    Iterator end() {
        return {std::end(*Container.Ptr()), {Mapper.Ptr()}};
    }

protected:
    TContainerStorage Container;
    TMapperStorage Mapper;
};


template <class TContainer, class TMapper>
class TOwningMappedIteratorRange : public TSimpleOwningMappedIteratorRange<TContainer, TMapper> {
public:
    using TBase = TSimpleOwningMappedIteratorRange<TContainer, TMapper>;

    using InternalIterator = typename TBase::InternalIterator;
    using Iterator = typename TBase::Iterator;
    using iterator = typename TBase::iterator;
    using value_type = typename TBase::value_type;
    using reference = typename TBase::reference;
    using difference_type = typename TBase::difference_type;

    using size_type = std::size_t;
    using ConstInternalIterator = typename TContainer::const_iterator;
    using ConstIterator = TMappedIterator<ConstInternalIterator, TMapper>;
    using const_iterator = ConstIterator;
    using const_reference = typename std::iterator_traits<const_iterator>::reference;

    TOwningMappedIteratorRange(TContainer&& container, TMapper mapper)
        : TSimpleOwningMappedIteratorRange<TContainer, TMapper>(std::move(container), std::move(mapper))
    {
    }

    using TBase::begin;
    using TBase::end;

    ConstIterator begin() const {
        return {std::begin(*this->Container.Ptr()), *this->Mapper.Ptr()};
    }

    ConstIterator end() const {
        return {std::end(*this->Container.Ptr()), *this->Mapper.Ptr()};
    }

    bool empty() const {
        return std::end(*this->Container.Ptr()) == std::begin(*this->Container.Ptr());
    }

    size_type size() const {
        return std::end(*this->Container.Ptr()) - std::begin(*this->Container.Ptr());
    }

    const_reference operator[](size_t at) const {
        Y_ASSERT(at < this->size());

        return *(this->begin() + at);
    }

    reference operator[](size_t at) {
        Y_ASSERT(at < this->size());

        return *(this->begin() + at);
    }
};

template <class TIterator, class TMapper>
TMappedIterator<TIterator, TMapper> MakeMappedIterator(TIterator iter, TMapper mapper) {
    return {iter, mapper};
}

template <class TIterator, class TMapper>
auto MakeMappedRange(TIterator begin, TIterator end, TMapper mapper) {
    return MakeIteratorRange(MakeMappedIterator(begin, mapper), MakeMappedIterator(end, mapper));
}

template <class TContainer, class TMapper>
auto MakeMappedRange(TContainer& container, TMapper mapper) {
    return MakeMappedRange(std::begin(container), std::end(container), mapper);
}

template <class TContainer, class TMapper>
auto MakeMappedRange(TContainer&& container, TMapper&& mapper) {
    return TOwningMappedIteratorRange<TContainer, TMapper>(std::forward<TContainer>(container), std::forward<TMapper>(mapper));
}

template <class TContainer, class TMapper>
auto MakeSimpleMappedRange(TContainer&& container, TMapper&& mapper) {
    return TSimpleOwningMappedIteratorRange<TContainer, TMapper>(std::forward<TContainer>(container), std::forward<TMapper>(mapper));
}
