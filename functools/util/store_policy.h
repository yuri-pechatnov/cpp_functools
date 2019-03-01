#pragma once

#include <utility>

template <class T>
struct TPtrPolicy {
    inline TPtrPolicy(T* t)
        : T_(t)
    {
    }

    inline T* Ptr() noexcept {
        return T_;
    }

    inline const T* Ptr() const noexcept {
        return T_;
    }

    T* T_;
};

template <class T>
struct TEmbedPolicy {
    template <typename... Args>
    inline TEmbedPolicy(Args&&... args)
        : T_(std::forward<Args>(args)...)
    {
    }

    inline T* Ptr() noexcept {
        return &T_;
    }

    inline const T* Ptr() const noexcept {
        return &T_;
    }

    T T_;
};


template <class TRefOrObject, bool IsReference = std::is_reference<TRefOrObject>::value>
struct TAutoEmbedOrPtrPolicy;

template <class TReference>
struct TAutoEmbedOrPtrPolicy<TReference, true> : TPtrPolicy<typename std::remove_reference<TReference>::type> {
    using TObject = typename std::remove_reference<TReference>::type;
    using TObjectStorage = TObject*;

    TAutoEmbedOrPtrPolicy(TReference& reference)
        : TPtrPolicy<TObject>(&reference)
    {
    }
};

template <class TObject_>
struct TAutoEmbedOrPtrPolicy<TObject_, false> : TEmbedPolicy<TObject_> {
    using TObject = TObject_;
    using TObjectStorage = TObject;

    TAutoEmbedOrPtrPolicy(TObject& object)
        : TEmbedPolicy<TObject>(std::move(object))
    {
    }

    TAutoEmbedOrPtrPolicy(TObject&& object)
        : TEmbedPolicy<TObject>(std::move(object))
    {
    }
};
