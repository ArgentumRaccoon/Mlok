#include "MlokMemory.h"
#include <cassert> // TODO: replace with custom assert

MlokAllocator::MlokAllocator(void* const inStart, const size_t inSize) noexcept
    : Start { inStart }
    , MemStats { inSize, 0, 0 }
{
    for (size_t TagId = 0; TagId < MEMORY_TAG_MAX; ++TagId)
    {
        MemStats.TaggedAllocations[TagId] = 0;
    }
}

MlokAllocator::MlokAllocator(MlokAllocator&& inAllocator) noexcept
    : Start { inAllocator.Start }
    , MemStats { inAllocator.MemStats }
{
    inAllocator.Start = nullptr;
    inAllocator.MemStats = { 0, 0, 0 };
    for (size_t TagId = 0; TagId < MEMORY_TAG_MAX; ++TagId)
    {
        inAllocator.MemStats.TaggedAllocations[TagId] = 0;
    }
}

MlokAllocator::~MlokAllocator() noexcept
{
    // TODO: replace with custom assert
    assert(MemStats.NumAllocations == 0 && MemStats.UsedBytes == 0);
}

MlokAllocator& MlokAllocator::operator=(MlokAllocator&& inAllocator) noexcept
{
    Start = inAllocator.Start;
    MemStats = inAllocator.MemStats;

    inAllocator.Start = nullptr;
    inAllocator.MemStats = { 0, 0, 0 };
    for (size_t TagId = 0; TagId < MEMORY_TAG_MAX; ++TagId)
    {
        inAllocator.MemStats.TaggedAllocations[TagId] = 0;
    }

    return *this;
}
