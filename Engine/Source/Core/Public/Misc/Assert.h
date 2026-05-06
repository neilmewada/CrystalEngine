#pragma once

#include "Misc/Exception.h"

#if PLATFORM_WINDOWS
#   define DEBUG_BREAK() __debugbreak()
#else
#   include <signal.h>
#   define DEBUG_BREAK() raise(SIGTRAP)
#endif

#include <cassert>

#if CE_BUILD_DEBUG
#   define CE_ASSERT(Condition, Message, ...)\
        do { if (!(Condition)) { CE_LOG(Critical, All, Message "", ##__VA_ARGS__); DEBUG_BREAK(); assert(false); } } while(0)
#else
#   define CE_ASSERT(Condition, Message, ...)\
        if (!(Condition)) { CE_LOG(Critical, All, Message "", ##__VA_ARGS__); assert(false); }
#endif

#define ASSERT(Condition, Message, ...) CE_ASSERT(Condition, Message, ##__VA_ARGS__)

