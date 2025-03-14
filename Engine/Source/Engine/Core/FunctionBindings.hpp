#pragma once

#include <utility>

#define ENG_BIND_FUNC(object, func) \
    [&](auto&&... args) -> decltype(auto) \
    { \
        return (object).func(::std::forward<decltype(args)>(args)...); \
    }
