#pragma once

#include "Defines.h"

#include <iostream>
#include <fstream>

namespace FileSystem
{
    MAPI bool Exists(const std::string& FilePath);
};

class MAPI FileHandle
{
    public:
        FileHandle(const std::string& inPath);
        ~FileHandle();

        bool Open(bool bBinaryMode);
        void Close();

        bool ReadLine(std::string& outLine);
        bool WriteLine(const std::string& inLine);
        bool ReadAllBytes(std::vector<char>& outBytes, size_t* outBytesRead); // TODO: custom allocator for bytes vector

    private:
        std::fstream Stream;

        std::string Path;
};