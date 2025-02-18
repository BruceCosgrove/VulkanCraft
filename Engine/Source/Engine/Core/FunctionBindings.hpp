#pragma once

#define ENG_BIND_CLASS_FUNC(func) \
    [this](auto&&... args) -> decltype(auto) \
    { \
        return this->func(std::forward<decltype(args)>(args)...); \
    }

#define ENG_BIND_MEMBER_FUNC(member, func) \
    [&](auto&&... args) -> decltype(auto) \
    { \
        return (member).func(std::forward<decltype(args)>(args)...); \
    }
