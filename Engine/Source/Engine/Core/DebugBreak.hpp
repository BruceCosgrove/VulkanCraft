#pragma once

#if ENG_CONFIG_DEBUG
    #if ENG_SYSTEM_WINDOWS
        #define ENG_DEBUG_BREAK() __debugbreak()
    #endif
#else
    #define ENG_DEBUG_BREAK() static_cast<void>(0)
#endif
