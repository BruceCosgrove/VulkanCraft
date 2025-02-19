#pragma once

#if ENG_ENABLE_ASSERTS || ENG_ENABLE_VERIFYS
    #include "Engine/Core/DebugBreak.hpp"
    #include "Engine/Core/Log.hpp"
    #include <string_view>

    #define _ENG_ASSERT_OR_VERIFY(dobreak, which, condition, ...) \
        do \
        { \
            if (!(condition)) \
            { \
                ENG_LOG_ERROR(#which " (" #condition ") failed at {}:{}", ::std::string_view(__FILE__).substr(::std::string_view(__FILE__).find("VulkanCraft") + 12ull), __LINE__); \
                __VA_OPT__(ENG_LOG_ERROR(__VA_ARGS__);) \
                dobreak \
            } \
        } \
        while (false)
#endif

#if ENG_ENABLE_ASSERTS
    #define ENG_ASSERT(condition, ...) _ENG_ASSERT_OR_VERIFY(ENG_DEBUG_BREAK();, Assertion, condition __VA_OPT__(,) __VA_ARGS__)
    #define ENG_ASSERT_NOBREAK(condition, ...) _ENG_ASSERT_OR_VERIFY(, Assertion, condition __VA_OPT__(,) __VA_ARGS__)
#else
    #define ENG_ASSERT(condition, ...) static_cast<void>(0)
    #define ENG_ASSERT_NOBREAK(condition, ...) static_cast<void>(0)
#endif

#if ENG_ENABLE_VERIFYS
    #define ENG_VERIFY(condition, ...) _ENG_ASSERT_OR_VERIFY(ENG_DEBUG_BREAK();, Verification, condition __VA_OPT__(,) __VA_ARGS__)
    #define ENG_VERIFY_NOBREAK(condition, ...) _ENG_ASSERT_OR_VERIFY(, Verification, condition __VA_OPT__(,) __VA_ARGS__)
#else
    #define ENG_VERIFY(condition, ...) condition
    #define ENG_VERIFY_NOBREAK(condition, ...) condition
#endif
