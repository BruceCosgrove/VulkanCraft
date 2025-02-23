#pragma once

#include "Engine/Core/AssertOrVerify.hpp"
#include <bit>
#include <concepts>

namespace eng
{
    template <typename BoundedEnumT, typename UT = std::underlying_type_t<BoundedEnumT>>
    concept BoundedEnumImplI =
        // UT is effectively a using declaration. Also enforces that MaskedEnumT is an enum.
        std::is_same_v<UT, std::underlying_type_t<BoundedEnumT>> &&
        static_cast<UT>(BoundedEnumT::None) == 0 &&
        static_cast<UT>(BoundedEnumT::_Begin) == static_cast<UT>(BoundedEnumT::None) + 1 &&
        static_cast<UT>(BoundedEnumT::_Count) == BoundedEnumT::_End - BoundedEnumT::_Begin &&
        static_cast<UT>(BoundedEnumT::_BitCount) == std::bit_width(static_cast<UT>(BoundedEnumT::_Count)) &&
        static_cast<UT>(BoundedEnumT::_Count) > 0;
    
    template <typename MaskedEnumT, typename UT = std::underlying_type_t<MaskedEnumT>>
    concept MaskedEnumImplI =
        // UT is effectively a using declaration. Also enforces that MaskedEnumT is an enum.
        std::is_same_v<UT, std::underlying_type_t<MaskedEnumT>> &&
        static_cast<UT>(MaskedEnumT::None) == 0 &&
        static_cast<UT>(MaskedEnumT::_Begin) == (1 << 0) &&
        static_cast<UT>(MaskedEnumT::_Mask) == (static_cast<UT>(MaskedEnumT::_End) << 1) - 3 &&
        static_cast<UT>(MaskedEnumT::_Count) == std::countr_one(static_cast<UT>(MaskedEnumT::_Mask)) &&
        MaskedEnumT::_BitCount == MaskedEnumT::_Count &&
        static_cast<UT>(MaskedEnumT::_Count) > 0;

    template <typename BoundedEnumT>
    concept BoundedEnumI =
        BoundedEnumImplI<BoundedEnumT> &&
        std::is_scoped_enum_v<BoundedEnumT> &&
        requires(BoundedEnumT e)
        {
            { e - e } -> std::same_as<std::underlying_type_t<BoundedEnumT>>;
            { +e } -> std::same_as<std::underlying_type_t<BoundedEnumT>>;
            { !e } -> std::same_as<bool>;
        };

    template <typename MaskedEnumT>
    concept MaskedEnumI =
        MaskedEnumImplI<MaskedEnumT> &&
        std::is_scoped_enum_v<MaskedEnumT> &&
        std::is_unsigned_v<std::underlying_type_t<MaskedEnumT>> &&
        requires(MaskedEnumT e)
        {
            { +e } -> std::same_as<std::underlying_type_t<MaskedEnumT>>;
            { ~e } -> std::same_as<MaskedEnumT>;
            { e & e } -> std::same_as<MaskedEnumT>;
            { e ^ e } -> std::same_as<MaskedEnumT>;
            { e | e } -> std::same_as<MaskedEnumT>;
            { !e } -> std::same_as<bool>;
        };

    template <typename BoundedClassEnumT, class ClassT>
    concept BoundedClassEnumI =
        BoundedEnumImplI<BoundedClassEnumT> &&
        !std::is_scoped_enum_v<BoundedClassEnumT> &&
        std::is_same_v<typename ClassT::Type, BoundedClassEnumT>&&
        std::is_class_v<ClassT>;

    template <typename MaskedClassEnumT, class ClassT>
    concept MaskedClassEnumI =
        MaskedEnumImplI<MaskedClassEnumT> &&
        !std::is_scoped_enum_v<MaskedClassEnumT> &&
        std::is_unsigned_v<std::underlying_type_t<MaskedClassEnumT>> &&
        std::is_same_v<typename ClassT::Type, MaskedClassEnumT> &&
        std::is_class_v<ClassT>;

    template <BoundedEnumI BoundedEnumT>
    constexpr bool IsBoundedEnumValid(BoundedEnumT e) noexcept
    {
        return BoundedEnumT::_Begin <= e and e < BoundedEnumT::_End;
    }

    template <MaskedEnumI MaskedEnumT>
    constexpr bool IsMaskedEnumValid(MaskedEnumT e) noexcept
    {
        return (MaskedEnumT::_Mask & e) == e;
    }

    template <class ClassT, BoundedClassEnumI<ClassT> BoundedClassEnumT>
    constexpr bool IsBoundedClassEnumValid(BoundedClassEnumT e) noexcept
    {
        return ClassT::_Begin <= e and e < ClassT::_End;
    }

    template <class ClassT, MaskedClassEnumI<ClassT> MaskedClassEnumT>
    constexpr bool IsMaskedClassEnumValid(MaskedClassEnumT e) noexcept
    {
        return (ClassT::_Mask & e) == e;
    }
    
    template <auto BitIn, auto BitOut>
    requires(eng::IsMaskedEnumValid(BitIn) and std::is_unsigned_v<decltype(BitOut)>)
    constexpr decltype(BitOut) TranslateMaskedEnum(decltype(BitIn) value) noexcept
    {
        using CT = std::common_type_t<decltype(+BitIn), decltype(BitOut)>;
        constexpr int bitShift = std::countr_zero(BitOut) - std::countr_zero(+BitIn);
        if constexpr (bitShift >= 0)
            return static_cast<decltype(BitOut)>(static_cast<CT>(value & BitIn) << bitShift);
        else /*if constexpr (bitShift < 0)*/
            return static_cast<decltype(BitOut)>(static_cast<CT>(value & BitIn) >> -bitShift);
    }
}

#define _ENG_DEFINE_BOUNDED_ENUM_IMPL(clazz, enumName, underlyingType, ...) \
    enum clazz enumName : underlyingType { \
        None = 0, \
        __VA_ARGS__ /* Requires trailing comma. */ \
        _End, \
        _Begin = None + 1, \
        _Count = _End - _Begin, \
        _BitCount = std::bit_width(_Count) \
    }

#define _ENG_DEFINE_MASKED_ENUM_IMPL(clazz, enumName, underlyingType, ...) \
    enum clazz enumName : underlyingType { \
        None = 0, \
        __VA_ARGS__ /* Requires trailing comma. */ \
        _End, \
        _Begin = 1 << 0, \
        _Mask = (_End << 1) - 3, \
        _Count = std::countr_one(_Mask), \
        _BitCount = _Count \
    }

#define ENG_DEFINE_BOUNDED_ENUM(enumName, underlyingType, ...) \
    _ENG_DEFINE_BOUNDED_ENUM_IMPL(class, enumName, underlyingType, __VA_ARGS__); \
    constexpr underlyingType operator-(enumName e1, enumName e2) noexcept \
    { return static_cast<underlyingType>(e1) - static_cast<underlyingType>(e2); } \
    constexpr underlyingType operator+(enumName e) noexcept \
    { return static_cast<underlyingType>(e); } \
    constexpr bool operator!(enumName e) noexcept \
    { return static_cast<underlyingType>(e) == static_cast<underlyingType>(enumName::None); } \
    static_assert(::eng::BoundedEnumI<enumName>, #enumName " does not satisfy BoundedEnumI's constraints.")

#define ENG_DEFINE_MASKED_ENUM(enumName, underlyingType, ...) \
    _ENG_DEFINE_MASKED_ENUM_IMPL(class, enumName, underlyingType, __VA_ARGS__); \
    constexpr underlyingType operator+(enumName e) noexcept \
    { return static_cast<underlyingType>(e); } \
    constexpr enumName operator~(enumName e) noexcept \
    { return static_cast<enumName>(~static_cast<underlyingType>(e) & static_cast<underlyingType>(enumName::_Mask)); } \
    constexpr enumName operator&(enumName e1, enumName e2) noexcept \
    { return static_cast<enumName>(static_cast<underlyingType>(e1) & static_cast<underlyingType>(e2)); } \
    constexpr enumName operator^(enumName e1, enumName e2) noexcept \
    { return static_cast<enumName>(static_cast<underlyingType>(e1) ^ static_cast<underlyingType>(e2)); } \
    constexpr enumName operator|(enumName e1, enumName e2) noexcept \
    { return static_cast<enumName>(static_cast<underlyingType>(e1) | static_cast<underlyingType>(e2)); } \
    constexpr enumName operator&=(enumName& e1, enumName e2) noexcept { return e1 = e1 & e2; } \
    constexpr enumName operator^=(enumName& e1, enumName e2) noexcept { return e1 = e1 ^ e2; } \
    constexpr enumName operator|=(enumName& e1, enumName e2) noexcept { return e1 = e1 | e2; } \
    constexpr bool operator!(enumName e) noexcept \
    { return static_cast<underlyingType>(e) == static_cast<underlyingType>(enumName::None); } \
    static_assert(::eng::MaskedEnumI<enumName>, #enumName " does not satisfy MaskedEnumI's constraints.")

// NOTE: Not an enum class. This is an unscoped enum designed to be nested in a class.
#define ENG_DEFINE_BOUNDED_CLASS_ENUM(className, underlyingType, ...) \
    _ENG_DEFINE_BOUNDED_ENUM_IMPL(, Type, underlyingType, __VA_ARGS__); \
    static_assert(::eng::BoundedClassEnumI<className::Type, className>, #className " does not satisfy BoundedClassEnumI's constraints.")

// NOTE: Not an enum class. This is an unscoped enum designed to be nested in a class.
#define ENG_DEFINE_MASKED_CLASS_ENUM(className, underlyingType, ...) \
    _ENG_DEFINE_MASKED_ENUM_IMPL(, Type, underlyingType, __VA_ARGS__); \
    static_assert(::eng::MaskedClassEnumI<className::Type, className>, #className " does not satisfy MaskedClassEnumI's constraints.")

// Common assert use case.

#define ENG_ASSERT_BOUNDED_ENUM_VALID(enumName, value) \
    ENG_ASSERT(::eng::IsBoundedEnumValid(value), "Unknown " #enumName ".");
#define ENG_ASSERT_BOUNDED_CLASS_ENUM_VALID(classEnumName, value) \
    ENG_ASSERT(::eng::IsBoundedClassEnumValid<classEnumName>(value), "Unknown " #classEnumName ".");
#define ENG_ASSERT_MASKED_ENUM_VALID(enumName, value) \
    ENG_ASSERT(::eng::IsMaskedEnumValid(value), "Unknown " #enumName ".");
#define ENG_ASSERT_MASKED_CLASS_ENUM_VALID(classEnumName, value) \
    ENG_ASSERT(::eng::IsMaskedClassEnumValid<classEnumName>(value), "Unknown " #classEnumName ".");
