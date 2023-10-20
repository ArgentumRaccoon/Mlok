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

// Allocators to use with STL containers
class MlokAllocator
{
    public:
        MlokAllocator(void* const inStart, const size_t inSize, const MemoryTag inTag) noexcept;
        MlokAllocator(const MlokAllocator& inAllocator) = delete;
        MlokAllocator(MlokAllocator&& inAllocator) noexcept;
        virtual ~MlokAllocator() noexcept;

        MlokAllocator& operator=(MlokAllocator& inAllocator) = delete;
        MlokAllocator& operator=(MlokAllocator&& inAllocator) noexcept;

        // Abstact class, leaving Allocate and Free implementation for the concrete allocators
        virtual void* Allocate(const size_t& inSize, const std::uintptr_t& Alignment = sizeof(std::intptr_t)) = 0;
        virtual void Free(void* const pData, size_t inSize) = 0;

        const void* GetStart() const noexcept { return Start; }
        const size_t& GetSize() const noexcept { return MemStats.Size; }
        const size_t& GetUsed() const noexcept { return MemStats.UsedBytes; }
        const size_t& GetNumAllocations() const noexcept { return MemStats.NumAllocations; };

        const size_t& GetTaggedAllocation(MemoryTag Tag) const noexcept { return MemStats.TaggedAllocations[Tag]; }

    protected:
        void* Start;
        MemoryTag Tag;

        struct MemStats
        {
            size_t Size;
            size_t NumAllocations;
            size_t UsedBytes;

            size_t TaggedAllocations[MEMORY_TAG_MAX];
        } MemStats;

        bool bUsedInitialAllocation = false;
};

class MlokLinearAllocator : public MlokAllocator
{
    public:
        MlokLinearAllocator(void* const inStart, const size_t inSize, const MemoryTag inTag) noexcept;
        MlokLinearAllocator(const MlokLinearAllocator& inAllocator) = delete;
        MlokLinearAllocator(MlokLinearAllocator&& inAllocator) noexcept;
        ~MlokLinearAllocator();

        MlokLinearAllocator& operator=(MlokLinearAllocator& inAllocator) = delete;
        MlokLinearAllocator& operator=(MlokLinearAllocator&& inAllocator) noexcept;

        virtual void* Allocate(const size_t& inSize, const std::uintptr_t& Alignment = sizeof(std::intptr_t)) noexcept override;
        virtual void Free(void* const pData, size_t inSize) noexcept override;        

        void* GetCurrent() const noexcept { return pCurrent; }

        virtual void Rewind(void* const pMark) noexcept;
        virtual void Clear() noexcept;

    protected:
        void* pCurrent;
};

template<typename T, typename AllocatorType>
class AllocatorSTLAdaptor
{
    public:
        using value_type = T;

        AllocatorSTLAdaptor() = delete;
        AllocatorSTLAdaptor(AllocatorType& inAllocator) noexcept
            : Allocator(inAllocator)
        { }

        template<typename U>
        AllocatorSTLAdaptor(const AllocatorSTLAdaptor<U, AllocatorType>& Other) noexcept
            : Allocator { Other.Allocator }
        { }

        [[nodiscard]] constexpr T* allocate(size_t Size)
        {
            return reinterpret_cast<T*>(Allocator.Allocate(Size * sizeof(T), alignof(T)));
        }

        constexpr void deallocate(T* Ptr, [[maybe_unused]] size_t Size) noexcept
        {
            Allocator.Free(Ptr, Size);
        }

        size_t MaxAllocationSize() const noexcept
        {
            return Allocator.GetSize();
        }

        bool operator==(const AllocatorSTLAdaptor<T, AllocatorType>& Other) const noexcept
        {
            return Allocator.GetStart() == Other.GetStart() &&
                   Allocator.GetSize() == Other.GetSize();
        }

        bool operator!=(const AllocatorSTLAdaptor<T, AllocatorType>& Other) const noexcept
        {
            return !(*this == Other);
        }

        AllocatorType& Allocator;
};

class MAPI MemorySystem
{
    
};