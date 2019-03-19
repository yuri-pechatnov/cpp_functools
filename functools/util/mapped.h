#pragma once

#include <iterator>
#include <iterator_range.h>
#include <store_policy.h>


namespace NPrivate {
    template <class TIterator>
    constexpr bool HasRandomAccess() {
        return std::is_same_v<typename std::iterator_traits<TIterator>::iterator_category,
                              std::random_access_iterator_tag>;
    }
};


template <class TIterator, class TMapper>
class TMappedIterator {
    using TSelf = TMappedIterator<TIterator, TMapper>;
    using TSrcPointerType = typename std::iterator_traits<TIterator>::reference;
    using TValue = decltype(std::declval<TMapper>()(std::declval<TSrcPointerType>()));
public:
    using difference_type = std::ptrdiff_t;
    using value_type = TValue;
    using reference = TValue&;
    using pointer = std::remove_reference_t<TValue>*;

    using iterator_category = std::conditional_t<NPrivate::HasRandomAccess<TIterator>(),
        std::random_access_iterator_tag, std::input_iterator_tag>;

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


template <class TContainer, class TMapper>
class TInputMappedRange {
protected:
    using TContainerStorage = TAutoEmbedOrPtrPolicy<TContainer>;
    using TMapperStorage = TAutoEmbedOrPtrPolicy<TMapper>;
    using TMapperWrapper = std::reference_wrapper<std::remove_reference_t<TMapper>>;
    using InternalIterator = decltype(std::begin(std::declval<TContainer&>()));
    using Iterator = TMappedIterator<InternalIterator, TMapperWrapper>;
public:
    using iterator = Iterator;
    using const_iterator = Iterator;
    using value_type = typename std::iterator_traits<iterator>::value_type;
    using reference = typename std::iterator_traits<iterator>::reference;
    using const_reference = typename std::iterator_traits<const_iterator>::reference;

    TInputMappedRange(TContainer&& container, TMapper&& mapper)
        : Container(std::forward<TContainer>(container))
        , Mapper(std::forward<TMapper>(mapper))
    {
    }

    Iterator begin() const {
        return {std::begin(*Container.Ptr()), {*Mapper.Ptr()}};
    }

    Iterator end() const {
        return {std::end(*Container.Ptr()), {*Mapper.Ptr()}};
    }

protected:
    mutable TContainerStorage Container;
    mutable TMapperStorage Mapper;
};


template <class TContainer, class TMapper>
class TRandomAccessMappedRange : public TInputMappedRange<TContainer, TMapper> {
    using TBase = TInputMappedRange<TContainer, TMapper>;
    using InternalIterator = typename TBase::InternalIterator;
    using Iterator = typename TBase::Iterator;
public:
    using iterator = typename TBase::iterator;
    using const_iterator = typename TBase::const_iterator;
    using value_type = typename TBase::value_type;
    using reference = typename TBase::reference;
    using const_reference = typename TBase::const_reference;

    using difference_type = typename std::iterator_traits<iterator>::difference_type;
    using size_type = std::size_t;

    TRandomAccessMappedRange(TContainer&& container, TMapper&& mapper)
        : TBase(std::forward<TContainer>(container), std::forward<TMapper>(mapper))
    {
    }

    using TBase::begin;
    using TBase::end;

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
auto MakeMappedRange(TContainer&& container, TMapper&& mapper) {
    if constexpr (NPrivate::HasRandomAccess<decltype(std::begin(container))>()) {
        return TRandomAccessMappedRange<TContainer, TMapper>(std::forward<TContainer>(container), std::forward<TMapper>(mapper));
    } else {
        return TInputMappedRange<TContainer, TMapper>(std::forward<TContainer>(container), std::forward<TMapper>(mapper));
    }
}
