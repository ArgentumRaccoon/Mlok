#pragma once

#include "Defines.h"

#define MASSERTIONS_ENABLED

#ifdef MASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define MDebugBreak() __debugbreak()
#else
#define MDebugBreak() __builtin_trap()
#endif

MAPI void ReportAssertionFailure(const char* Expression, const char* Message, const char* File, int32_t Line);

#define M_ASSERT(expr)                                                  \
        {                                                               \
            if (expr) {}                                                \
            else                                                        \
            {                                                           \
                ReportAssertionFailure(#expr, "", __FILE__, __LINE__);  \
                MDebugBreak();                                           \
            }                                                           \
        }

#define M_ASSERT_MSG(expr, message)                                             \
        {                                                                       \
            if (expr) {}                                                        \
            else                                                                \
            {                                                                   \
                ReportAssertionFailure(#expr, #message, __FILE__, __LINE__);    \
                MDebugBreak();                                                   \
            }                                                                   \
        }

#ifndef NDEBUG
#define M_ASSERT_DEBUG(expr)                                            \
        {                                                               \
            if (expr) {}                                                \
            else                                                        \
            {                                                           \
                ReportAssertionFailure(#expr, "", __FILE__, __LINE__);  \
                MDebugBreak();                                           \
            }                                                           \
        }
#else
#define M_ASSERT_DEBUG(expr)
#endif
#else
#define M_ASSERT(expr)
#define M_ASSERT_MSG(expr, message)
#define M_ASSERT_DEBUG(expr)
#endif