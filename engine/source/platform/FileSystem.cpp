#include "FileSystem.h"

#include <filesystem>

bool FileSystem::Exists(const std::string& FilePath)
{
    return std::filesystem::exists(std::filesystem::path(FilePath));
}

FileHandle::FileHandle(const std::string& inPath)
    : Path(inPath)
{

}

FileHandle::~FileHandle()
{
    Close();
}

bool FileHandle::Open(bool bBinaryMode)
{
    if (!FileSystem::Exists(Path) || Stream.is_open())
    {
        return false;
    }

    std::ios::openmode Mode = std::ios::in | std::ios::out | std::ios::ate;
    if (bBinaryMode)
    {
        Mode |= std::ios::binary;
    }

    Stream.open(Path, Mode);

    return true;
}

void FileHandle::Close()
{
    if (Stream.is_open())
    {
        Stream.close();
    }
}

bool FileHandle::ReadLine(std::string& outLine)
{
    if (!Stream.is_open())
    {
        return false;
    }

    if (std::getline(Stream, outLine))
    {
        return true;
    }

    return false;
}

bool FileHandle::WriteLine(const std::string& inLine)
{
    if (!Stream.is_open())
    {
        return false;
    }

    if (!inLine.empty())
    {
        Stream << inLine << std::endl;
    }

    return true;
}

bool FileHandle::ReadAllBytes(std::vector<char>& outBytes, size_t* outBytesRead)
{
    outBytes.clear();

    Stream.seekg(0, std::ios::end);
    *outBytesRead = Stream.tellg();
    Stream.seekg(0, std::ios::beg);

    outBytes.resize(*outBytesRead);

    Stream.read(outBytes.data(), *outBytesRead);

    return true;
}