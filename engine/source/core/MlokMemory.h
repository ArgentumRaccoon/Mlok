#pragma once

#include "Defines.h"

typedef enum MemoryTag
{
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX
} MemoryTag;

class MlokAllocator
{
    public:
        MlokAllocator(void* const inStart, const size_t inSize) noexcept;
        MlokAllocator(const MlokAllocator& inAllocator) = delete;
        MlokAllocator(MlokAllocator&& inAllocator) noexcept;
        virtual ~MlokAllocator() noexcept;

        MlokAllocator& operator=(MlokAllocator& inAllocator) = delete;
        MlokAllocator& operator=(MlokAllocator&& inAllocator) noexcept;

        // Abstact class, leaving Allocate and Free implementation for the concrete allocators
        virtual void* Allocate(const size_t& inSize, MemoryTag Tag, const std::uintptr_t& Alignment = sizeof(std::intptr_t)) = 0;
        virtual void Free(void* const pData, size_t inSize, MemoryTag Tag) = 0;

        const void* GetStart() const noexcept { return Start; }
        const size_t& GetSize() const noexcept { return MemStats.Size; }
        const size_t& GetUsed() const noexcept { return MemStats.UsedBytes; }
        const size_t& GetNumAllocations() const noexcept { return MemStats.NumAllocations; };

        const size_t& GetTaggedAllocation(MemoryTag Tag) const noexcept { return MemStats.TaggedAllocations[Tag]; }

    protected:
        void* Start;

        struct MemStats
        {
            size_t Size;
            size_t NumAllocations;
            size_t UsedBytes;

            size_t TaggedAllocations[MEMORY_TAG_MAX];
        } MemStats;
};

class MlokLinearAllocator : public MlokAllocator
{
    public:
        MlokLinearAllocator(void* const inStart, const size_t inSize) noexcept;
        MlokLinearAllocator(const MlokLinearAllocator& inAllocator) = delete;
        MlokLinearAllocator(MlokLinearAllocator&& inAllocator) noexcept;
        ~MlokLinearAllocator();

        MlokLinearAllocator& operator=(MlokLinearAllocator& inAllocator) = delete;
        MlokLinearAllocator& operator=(MlokLinearAllocator&& inAllocator) noexcept;

        virtual void* Allocate(const size_t& inSize, MemoryTag Tag, const std::uintptr_t& Alignment = sizeof(std::intptr_t)) noexcept override;
        virtual void Free(void* const pData, size_t inSize, MemoryTag Tag) noexcept override;        

        void* GetCurrent() const noexcept { return pCurrent; }

        virtual void Rewind(void* const pMark) noexcept;
        virtual void Clear() noexcept;

    protected:
        void* pCurrent;
};