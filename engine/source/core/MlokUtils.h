#pragma once

#include "Defines.h"

#include <cstring>

namespace MlokUtils
{
    // TODO: replace with std::format after switching to c++20
    template<typename... TArgs>
    MAPI std::string StringFormat(const std::string& Format, TArgs&&... Args)
    {
        const char* const CFormat = Format.c_str();
        int SizeS = std::snprintf(nullptr, 0, CFormat, Args...) + 1;
        if (SizeS <= 0)
        {
            return std::string {};
        }

        size_t Size = static_cast<size_t>(SizeS);
        std::unique_ptr<char[]> Buffer(new char[Size]);
        std::snprintf(Buffer.get(), Size, CFormat, Args...);
        return std::string(Buffer.get(), Buffer.get() + Size - 1);
    }

    MAPI MINLINE bool StringsAreEqual(const char* Str1, const char* Str2)
    {
        return strcmp(Str1, Str2);
    }

    MAPI MINLINE float BytesToKib(size_t Bytes)
    {
        return (float)Bytes / 1024.f;
    }

    MAPI MINLINE float BytesToMib(size_t Bytes)
    {
        return (float)Bytes / 1024.f / 1024.f;
    }

    MAPI MINLINE float BytesToGib(size_t Bytes)
    {
        return (float)Bytes / 1024.f / 1024.f / 1024.f;
    }

    template<typename T>
    MAPI MINLINE T Clamp(const T Value, const T Min, const T Max)
    {
        return (Value <= Min) ? Min : (Value >= Max) ? Max : Value;
    }
}