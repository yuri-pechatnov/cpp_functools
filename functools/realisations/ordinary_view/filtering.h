#pragma once

#include <util/generic/store_policy.h>

#include <iterator>
#include <limits>
#include <tuple>


namespace NPrivate {

    template <typename TContainer, typename TCondition>
    struct TFilterer {
    private:
        using TContainerStorage = TAutoEmbedOrPtrPolicy<TContainer>;
        using TConditionStorage = TAutoEmbedOrPtrPolicy<TCondition>;
        using TValue = decltype(*std::begin(std::declval<TContainer&>()));
        using TIteratorState = decltype(std::begin(std::declval<TContainer&>()));
        using TSentinelState = decltype(std::end(std::declval<TContainer&>()));

        static constexpr bool TrivialSentinel = std::is_same_v<TIteratorState, TSentinelState>;

        struct TIterator;
        struct TSentinelCandidate {
            TSentinelState Iterator_;
        };
        using TSentinel = std::conditional_t<TrivialSentinel, TIterator, TSentinelCandidate>;

        struct TIterator {
            using difference_type = std::ptrdiff_t;
            using value_type = TValue;
            using pointer = std::remove_reference_t<TValue>*;
            using reference = TValue&;
            using iterator_category = std::input_iterator_tag;

            TValue operator*() {
                return *Iterator_;
            }
            TValue operator*() const {
                return *Iterator_;
            }
            void operator++() {
                do {
                    ++Iterator_;
                    if (!(Iterator_ != std::end(*Container_))) {
                        NotFinished = false;
                        return;
                    }
                } while (!(*Condition_)(*Iterator_));
            }
            bool operator!=(const TSentinel& other) const {
                if (other.NotFinished) {
                    return Iterator_ != other.Iterator_;
                } else {
                    return NotFinished;
                }
            }
            bool operator==(const TSentinel& other) const {
                return !(*this != other);
            }

            bool NotFinished;
            TIteratorState Iterator_;
            std::remove_reference_t<TContainer>* Container_;
            std::remove_reference_t<TCondition>* Condition_;
        };
    public:
        using iterator = TIterator;
        using const_iterator = TIterator;

        TIterator begin() const {
            auto first = std::begin(*Storage_.Ptr());
            while (first != std::end(*Storage_.Ptr()) && !(*Condition_.Ptr())(*first)) {
                ++first;
            }
            if (first != std::end(*Storage_.Ptr())) {
                return {true, first, Storage_.Ptr(), Condition_.Ptr()};
            } else {
                return {false, first, Storage_.Ptr(), Condition_.Ptr()};
            }
        }

        TSentinel end() const {
            if constexpr (TrivialSentinel) {
                return TIterator{false, std::end(*Storage_.Ptr()), Storage_.Ptr(), Condition_.Ptr()};
            } else {
                return TSentinel{std::end(*Storage_.Ptr())};
            }
        }

        mutable TConditionStorage Condition_;
        mutable TContainerStorage Storage_;
    };

}

template <typename TContainerOrRef, typename TConditionOrRef>
auto Filter(TConditionOrRef&& condition, TContainerOrRef&& container) {
    return NPrivate::TFilterer<TContainerOrRef, TConditionOrRef>{
            std::forward<TConditionOrRef>(condition), std::forward<TContainerOrRef>(container)};
}



