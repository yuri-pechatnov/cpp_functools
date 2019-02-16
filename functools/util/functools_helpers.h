#pragma once

#include <iterator_range.h>
#include <store_policy.h>

#include <optional>
#include <memory>
#include <cstdint>

namespace NFuncTools::NHelpers {

    //! Common end for TGeneratorRange
    struct TRangeSentinel {
    };

    //! Easy way to make an iterator range with one lambda function
    //! Supports a minimal set of operations to use in range-based for
    template <typename TGenerator>
    class TGeneratorRangeIterator {
    public:
        enum class EQueryType {
            Next,
            GetCurrent,
            IsNotEnd
        };

        template <EQueryType type>
        struct TQueryType {
            static constexpr EQueryType Type = type;
            static constexpr bool Next = (type == EQueryType::Next);
            static constexpr bool GetCurrent = (type == EQueryType::GetCurrent);
            static constexpr bool IsNotEnd = (type == EQueryType::IsNotEnd);
        };

        TGeneratorRangeIterator(const TGenerator& generator)
            : Generator(generator)
        {
        }

        TGeneratorRangeIterator(TGenerator&& generator)
            : Generator(std::move(generator))
        {
        }

        void operator++() {
            Generator(TQueryType<EQueryType::Next>{});
        }
        decltype(auto) operator*() {
            return Generator(TQueryType<EQueryType::GetCurrent>{});
        }
        bool operator!=(TRangeSentinel) {
            return Generator(TQueryType<EQueryType::IsNotEnd>{});
        }

    protected:
        TGenerator Generator;
    };


    template <typename TGeneratorCreator>
    class TGeneratorRange {
    public:
        TGeneratorRange(TGeneratorCreator&& generatorCreator)
            : GeneratorCreator(std::move(generatorCreator))
        {
        }

        auto begin() {
            return GeneratorCreator();
        }

        TRangeSentinel end() {
            return TRangeSentinel{};
        }
    protected:
        TGeneratorCreator GeneratorCreator;
    };



    //! For some functions (e.g. cartesian_product) we need copy assignable iterators, though
    //! I introduce a holder that keeps object itself for objects supporting copy assigning
    //! and keeps std::unique_ptr for other objects
    template <typename TObject>
    struct TCopyAssignableHolder {
        static constexpr bool IsCopyAssignable = std::is_copy_assignable_v<TObject>;
        using TStorage = std::conditional_t<IsCopyAssignable, TObject, std::optional<TObject>>;

        TCopyAssignableHolder(const TObject& object)
            : Storage(object)
        {
        }

        TCopyAssignableHolder(TObject&& object)
            : Storage(std::move(object))
        {
        }

        TCopyAssignableHolder(const TCopyAssignableHolder<TObject>& holder)
            : TCopyAssignableHolder(holder.Get())
        {
        }

        TCopyAssignableHolder(TCopyAssignableHolder<TObject>&& holder)
            : TCopyAssignableHolder(std::move(holder.Get()))
        {
        }

        void operator=(const TCopyAssignableHolder<TObject>& holder) {
            if constexpr (IsCopyAssignable) {
                Storage = holder.Get();
            } else {
                Storage.emplace(holder.Get());
            }
        }

        void operator=(TCopyAssignableHolder<TObject>&& holder) {
            if constexpr (IsCopyAssignable) {
                Storage = std::move(holder.Get());
            } else {
                Storage.emplace(std::move(holder.Get()));
            }
        }

        TObject& Get() {
            if constexpr (IsCopyAssignable) {
                return Storage;
            } else {
                return *Storage;
            }
        }

        const TObject& Get() const {
            if constexpr (IsCopyAssignable) {
                return Storage;
            } else {
                return *Storage;
            }
        }

    protected:
        TStorage Storage;
    };

}
