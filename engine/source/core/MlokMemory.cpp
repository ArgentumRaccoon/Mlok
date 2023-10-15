#include "MlokMemory.h"
#include <cassert> // TODO: replace with custom assert

inline size_t AlignForwardAdjustment(const void* const Ptr, const size_t& Alignment) noexcept
{
    const auto PtrCast = reinterpret_cast<std::uintptr_t>(Ptr);
    const auto Aligned = (PtrCast - 1u + Alignment) & (-Alignment);
    return Aligned - PtrCast;
}

inline void* PtrAdd(const void* const Ptr, const std::uintptr_t& Amount) noexcept
{
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(Ptr) + Amount);
}

MlokAllocator::MlokAllocator(void* const inStart, const size_t inSize, const MemoryTag inTag) noexcept
    : Start { inStart }
    , MemStats { inSize, 0, 0 }
    , Tag { inTag }
{
}

MlokAllocator::MlokAllocator(MlokAllocator&& inAllocator) noexcept
    : Start { inAllocator.Start }
    , MemStats { inAllocator.MemStats }
    , Tag { inAllocator.Tag }
{
    inAllocator.Start = nullptr;
    inAllocator.MemStats = { 0, 0, 0 };
    inAllocator.Tag = MemoryTag::MEMORY_TAG_MAX;
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

MlokLinearAllocator::MlokLinearAllocator(void* const inStart, const size_t inSize, const MemoryTag inTag) noexcept
    : MlokAllocator(inStart, inSize, inTag)
    , pCurrent { const_cast<void*>(Start) }
{

}

MlokLinearAllocator::MlokLinearAllocator(MlokLinearAllocator&& inAllocator) noexcept
    : MlokAllocator(std::move(inAllocator))
    , pCurrent { inAllocator.pCurrent }
{
    inAllocator.pCurrent = nullptr;
}

MlokLinearAllocator::~MlokLinearAllocator()
{
    Clear();
}

MlokLinearAllocator& MlokLinearAllocator::operator=(MlokLinearAllocator&& inAllocator) noexcept
{
    MlokAllocator::operator=(std::move(inAllocator));
    pCurrent = inAllocator.pCurrent;
    inAllocator.pCurrent = nullptr;
    return *this;
}

void* MlokLinearAllocator::Allocate(const size_t& inSize, const std::uintptr_t& Alignment) noexcept
{
    assert(inSize > 0 && Alignment > 0);

    size_t Adjust = AlignForwardAdjustment(pCurrent, Alignment);

    void* AlignedAddr = PtrAdd(pCurrent, Adjust);
    
    pCurrent = PtrAdd(AlignedAddr, inSize);
    MemStats.UsedBytes = reinterpret_cast<std::uintptr_t>(pCurrent) - reinterpret_cast<std::uintptr_t>(Start);

    ++(MemStats.NumAllocations);

    return AlignedAddr;
}

void MlokLinearAllocator::Free(void* const pData, size_t inSize) noexcept
{
    // No direct Free operation for LinearAllocator (using Rewind and Clear instead)
}

void MlokLinearAllocator::Rewind(void* const pMark) noexcept
{
    assert(pCurrent >= pMark && Start <= pMark);

    pCurrent = pMark;
    MemStats.UsedBytes = reinterpret_cast<std::uintptr_t>(pCurrent) - reinterpret_cast<std::uintptr_t>(Start);
}

void MlokLinearAllocator::Clear() noexcept
{
    MemStats.NumAllocations = 0;
    MemStats.UsedBytes = 0;
    pCurrent = Start;
}
