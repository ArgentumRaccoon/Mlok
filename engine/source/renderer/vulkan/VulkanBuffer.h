#pragma once

#include "VulkanTypes.inl"

class VulkanContext;

class VulkanBuffer
{
    public:
        VulkanBuffer();
        VulkanBuffer(VulkanContext* Context,
                     size_t Size,
                     vk::MemoryPropertyFlags MemoryPropertyFlags,
                     vk::BufferUsageFlags Usage,
                     bool bBindOnCreate);
        ~VulkanBuffer();

        MINLINE vk::Buffer* Get() { return &Handle; }

        bool Create(VulkanContext* Context,
                    size_t Size,
                    vk::MemoryPropertyFlags MemoryPropertyFlags,
                    vk::BufferUsageFlags Usage,
                    bool bBindOnCreate);
        void Destroy();

        bool Resize(size_t NewSize, vk::Queue Queue, vk::CommandPool Pool);

        void Bind(size_t Offset);
        void* LockMemory(size_t Offset, size_t Size, vk::MemoryMapFlags Flags);
        void UnlockMemory();

        void LoadData(size_t Offset, size_t Size, vk::MemoryMapFlags Flags, const void* Data);

        static void Copy(VulkanContext* Context,
                         vk::CommandPool Pool,
                         vk::Fence Fence,
                         vk::Queue Queue,
                         vk::Buffer Source,
                         size_t SourceOffset,
                         vk::Buffer Dest,
                         size_t DestOffset,
                         size_t Size);

    private:
        VulkanContext* Context;

        vk::Buffer Handle;
        size_t TotalSize;
        vk::BufferUsageFlags Usage;
        bool bIsLocked;
        vk::DeviceMemory Memory;
        int32_t MemoryIndex;
        vk::MemoryPropertyFlags MemoryPropertyFlags;
};