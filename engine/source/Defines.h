#pragma once

#include <cstdint>

// Properly define static assertions.
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) 
#define MPLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define MPLATFORM_LINUX 1
#else
#error "Unknown platform!"
#endif

#ifdef MEXPORT
// Exports
#ifdef _MSC_VER
#define MAPI __declspec(dllexport)
#else
#define MAPI __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define MAPI __declspec(dllimport)
#else
#define MAPI
#endif
#endif

#ifdef _MSC_VER
#define MINLINE __forceinline
#define MNOINLINE __declspec(noinline)
#else
#define MINLINE static inline
#define MNOINLINE __attribute__((noinline))
#endif