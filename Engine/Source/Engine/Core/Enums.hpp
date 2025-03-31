#pragma once

#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Core/ForEach.hpp"
#include <spdlog/fmt/bundled/format.h>
#include <array>
#include <bit>
#include <concepts>
#include <compare>

namespace eng::detail
{
    template <typename BoundedEnumT>
    concept BoundedEnumI =
        std::is_class_v<BoundedEnumT> and
        std::is_enum_v<decltype(BoundedEnumT::None)> and
        !std::is_scoped_enum_v<decltype(BoundedEnumT::None)> and
        std::is_unsigned_v<std::underlying_type_t<decltype(BoundedEnumT::None)>> and
        BoundedEnumT::None == 0 and
        BoundedEnumT::_Begin == BoundedEnumT::None + 1 and
        BoundedEnumT::_Count == BoundedEnumT::_End - BoundedEnumT::_Begin and
        BoundedEnumT::_BitCount == std::bit_width<std::underlying_type_t<decltype(BoundedEnumT::None)>>(BoundedEnumT::_Count) and
        BoundedEnumT::_Count > 0 and
        requires(BoundedEnumT e)
        {
            { +e } -> std::same_as<std::underlying_type_t<decltype(BoundedEnumT::None)>>;
            { !e } -> std::same_as<bool>;
            { e == e } -> std::same_as<bool>;
            { e <=> e } -> std::same_as<std::strong_ordering>;
        };
    
    template <typename MaskedEnumT>
    concept MaskedEnumI =
        std::is_class_v<MaskedEnumT> and
        std::is_enum_v<decltype(MaskedEnumT::None)> and
        !std::is_scoped_enum_v<decltype(MaskedEnumT::None)> and
        std::is_unsigned_v<std::underlying_type_t<decltype(MaskedEnumT::None)>> and
        MaskedEnumT::None == 0 and
        MaskedEnumT::_Begin == 1 << 0 and
        MaskedEnumT::_Mask == (MaskedEnumT::_End << 1) - 3 and
        MaskedEnumT::_Count == std::countr_one<std::underlying_type_t<decltype(MaskedEnumT::None)>>(MaskedEnumT::_Mask) and
        MaskedEnumT::_BitCount == MaskedEnumT::_Count and
        MaskedEnumT::_Count > 0 and
        requires(MaskedEnumT e)
        {
            { +e } -> std::same_as<std::underlying_type_t<decltype(MaskedEnumT::None)>>;
            { !e } -> std::same_as<bool>;
            { e == e } -> std::same_as<bool>;
            { e <=> e } -> std::same_as<std::strong_ordering>;
            { ~e } -> std::same_as<MaskedEnumT>;
            { e & e } -> std::same_as<MaskedEnumT>;
            { e ^ e } -> std::same_as<MaskedEnumT>;
            { e | e } -> std::same_as<MaskedEnumT>;
            { e &= e } -> std::same_as<MaskedEnumT&>;
            { e ^= e } -> std::same_as<MaskedEnumT&>;
            { e |= e } -> std::same_as<MaskedEnumT&>;
        };

#define _ENG_ENUM_TO_STRING_IMPL(e) \
    #e,
#define _ENG_ENUM_BIT_IMPL(e) \
    _##e##Bit,
#define _ENG_ENUM_VALUE_IMPL(e) \
    e = 1 << _##e##Bit,
#define _ENG_ENUM_MAX_NAME_LENGTH_IMPL(enumName, e) \
    sizeof(#e) +
}

// Use this to define a bounded enum.
#define ENG_DEFINE_BOUNDED_ENUM(enumName, underlyingType, ...) \
    struct enumName { \
        enum : underlyingType { \
            None = 0, \
            __VA_ARGS__ /* Requires trailing comma. */ \
            _End, \
            _Begin = None + 1, \
            _Count = _End - _Begin, \
            _BitCount = ::std::bit_width(_Count) \
        }; \
        static constexpr ::eng::string_view Name = #enumName; \
        static constexpr auto Names = ::std::to_array<::eng::string_view const>({ \
            ENG_FOR_EACH(_ENG_ENUM_TO_STRING_IMPL, __VA_ARGS__) \
        }); \
        constexpr string_view ToString() const noexcept { return Names.at(m_Value - _Begin); } \
        constexpr enumName() noexcept = default; \
        constexpr enumName(underlyingType value) noexcept : m_Value(value) {} \
        constexpr underlyingType operator+() const noexcept { return m_Value; } \
        constexpr explicit operator bool() const noexcept { return m_Value; } \
        constexpr bool operator!() const noexcept { return !m_Value; } \
        friend constexpr bool operator==(enumName lhs, enumName rhs) noexcept { return lhs.m_Value == rhs.m_Value; } \
        friend constexpr ::std::strong_ordering operator<=>(enumName lhs, enumName rhs) noexcept { return lhs.m_Value <=> rhs.m_Value; } \
    private: \
        underlyingType m_Value = enumName::None; \
    }; \
    static_assert(::eng::detail::BoundedEnumI<enumName>, #enumName " does not satisfy BoundedEnumI's constraints.")

// Use this to define a masked enum.
#define ENG_DEFINE_MASKED_ENUM(enumName, underlyingType, ...) \
    struct enumName { \
        enum : underlyingType { \
            ENG_FOR_EACH(_ENG_ENUM_BIT_IMPL, __VA_ARGS__) \
            _Count, \
            None = 0, \
            ENG_FOR_EACH(_ENG_ENUM_VALUE_IMPL, __VA_ARGS__) \
            _End, \
            _Begin = 1 << 0, \
            _Mask = (_End << 1) - 3, \
            _BitCount = _Count \
        }; \
        static constexpr ::eng::string_view Name = #enumName; \
        static constexpr auto Names = ::std::to_array<::eng::string_view const>({ \
            ENG_FOR_EACH(_ENG_ENUM_TO_STRING_IMPL, __VA_ARGS__) \
        }); \
        constexpr ::eng::string ToString() { \
            if (m_Value == None) \
                return "None"; \
            \
            string mask; \
            mask.reserve(MaxNameLength); \
            \
            underlyingType index = std::countr_zero(m_Value); \
            mask += Names.at(index); \
            \
            auto next = [&] { index += std::countr_zero<underlyingType>(m_Value >> (index + 1)) + 1; }; \
            \
            for (next(); index < +_Count; next()) \
                (mask += '|') += Names.at(index); \
            return mask; \
        } \
        static constexpr ::eng::u64 MaxNameLength = ENG_FOR_EACH_ZIP1(_ENG_ENUM_MAX_NAME_LENGTH_IMPL, enumName, __VA_ARGS__) -1; \
        constexpr enumName() noexcept = default; \
        constexpr enumName(underlyingType value) noexcept : m_Value(value & _Mask) {} \
        constexpr underlyingType operator+() const noexcept { return m_Value; } \
        constexpr explicit operator bool() const noexcept { return m_Value; } \
        constexpr bool operator!() const noexcept { return !m_Value; } \
        friend constexpr bool operator==(enumName lhs, enumName rhs) noexcept { return lhs.m_Value == rhs.m_Value; } \
        friend constexpr ::std::strong_ordering operator<=>(enumName lhs, enumName rhs) noexcept { return lhs.m_Value <=> rhs.m_Value; } \
        constexpr enumName operator~() noexcept { return ~m_Value & _Mask; } \
        friend constexpr enumName operator&(enumName lhs, enumName rhs) noexcept { return lhs.m_Value & rhs.m_Value; } \
        friend constexpr enumName operator^(enumName lhs, enumName rhs) noexcept { return lhs.m_Value ^ rhs.m_Value; } \
        friend constexpr enumName operator|(enumName lhs, enumName rhs) noexcept { return lhs.m_Value | rhs.m_Value; } \
        friend constexpr enumName& operator&=(enumName& lhs, enumName rhs) noexcept { lhs.m_Value &= rhs.m_Value; return lhs; } \
        friend constexpr enumName& operator^=(enumName& lhs, enumName rhs) noexcept { lhs.m_Value ^= rhs.m_Value; return lhs; } \
        friend constexpr enumName& operator|=(enumName& lhs, enumName rhs) noexcept { lhs.m_Value |= rhs.m_Value; return lhs; } \
        constexpr bool HasAny(enumName bits) const noexcept { return (m_Value & bits) != 0; } \
        constexpr bool HasAll(enumName bits) const noexcept { return (m_Value & bits) == bits; } \
        constexpr bool HasOnly(enumName bits) const noexcept { return m_Value == bits; } \
    private: \
        underlyingType m_Value = None; \
    }; \
    static_assert(::eng::detail::MaskedEnumI<enumName>, #enumName " does not satisfy MaskedEnumI's constraints.")

// Validity checks
namespace eng
{
    template <detail::BoundedEnumI BoundedEnumT>
    constexpr bool IsBoundedEnumValid(BoundedEnumT e) noexcept
    {
        return BoundedEnumT::_Begin <= +e and +e < BoundedEnumT::_End;
    }

    template <detail::MaskedEnumI MaskedEnumT>
    constexpr bool IsMaskedEnumValid(MaskedEnumT e) noexcept
    {
        return (MaskedEnumT::_Mask & e) == e;
    }

    // Common assert use cases.

#define ENG_ASSERT_BOUNDED_ENUM_VALID(enumName, value) \
    ENG_ASSERT(::eng::IsBoundedEnumValid(value), "Unknown {}.", #enumName)
#define ENG_ASSERT_MASKED_ENUM_VALID(enumName, value) \
    ENG_ASSERT(::eng::IsMaskedEnumValid(value), "Unknown {}.", #enumName)
}

// Formatting

template <eng::detail::BoundedEnumI BoundedEnumT>
struct fmt::formatter<BoundedEnumT>
{
    constexpr auto parse(format_parse_context& context) { return context.end(); }

    auto format(BoundedEnumT e, fmt::format_context& context) const
    {
        return fmt::format_to(context.out(), "{}", e.ToString());
    }
};

template <eng::detail::MaskedEnumI MaskedEnumT>
struct fmt::formatter<MaskedEnumT>
{
    constexpr auto parse(format_parse_context& context) { return context.end(); }

    auto format(MaskedEnumT e, fmt::format_context& context) const
    {
        return fmt::format_to(context.out(), "{}", e.ToString());
    }
};
