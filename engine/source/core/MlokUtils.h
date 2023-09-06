#pragma once

#include "Defines.h"

#include <string>

namespace MlokUtils
{
    // TODO: replace with std::format after switching to c++20
    template<typename... TArgs>
    MAPI std::string StringFormat(const std::string& Format, TArgs&&... Args)
    {
        int SizeS = std::snprintf(nullptr, 0, Format.c_str(), Args...) + 1;
        if (SizeS <= 0)
        {
            return std::string {};
        }

        size_t Size = static_cast<size_t>(SizeS);
        std::unique_ptr<char[]> Buffer(new char[Size]);
        std::snprintf(Buffer.get(), Size, Format.c_str(), Args...);
        return std::string(Buffer.get(), Buffer.get() + Size - 1);
    }
}