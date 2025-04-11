#pragma once

#define ENG_FALLTHROUGH [[fallthrough]]

#define ENG_NO_DISCARD [[nodiscard]]

#define ENG_LIKELY [[likely]]
#define ENG_UNLIKELY [[unlikely]]

#if _MSVC_LANG
    #define ENG_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
    #define ENG_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
