#include "VulkanBuffer.h"

#include "VulkanContext.h"
#include "VulkanCommandBuffer.h"
#include "VulkanUtils.h"

#include "core/Logger.h"
#include "platform/Platform.h"

VulkanBuffer::VulkanBuffer()
{

}

VulkanBuffer::VulkanBuffer(VulkanContext* inContext,
                           size_t inSize,
                           vk::MemoryPropertyFlagBits inMemoryPropertyFlags,
                           vk::BufferUsageFlagBits inUsage,
                           bool bBindOnCreate)
    : Context { inContext }
    , TotalSize { inSize }
    , MemoryPropertyFlags { inMemoryPropertyFlags }
    , Usage { inUsage }
{
    Create(inContext, inSize, inMemoryPropertyFlags, inUsage, bBindOnCreate);
}

VulkanBuffer::~VulkanBuffer()
{
    Destroy();
}

bool VulkanBuffer::Create(VulkanContext* inContext,
                          size_t inSize,
                          vk::MemoryPropertyFlagBits inMemoryPropertyFlags,
                          vk::BufferUsageFlagBits inUsage,
                          bool bBindOnCreate)
{
    if (Context != inContext)
    {
        Context = inContext;
    }
    
    if (TotalSize != inSize)
    {
        TotalSize = inSize;
    }

    if (MemoryPropertyFlags != inMemoryPropertyFlags)
    {
        MemoryPropertyFlags = inMemoryPropertyFlags;
    }

    if (Usage != inUsage)
    {
        Usage = inUsage;
    }

    vk::BufferCreateInfo CreateInfo {};
    CreateInfo.setSize(TotalSize)
              .setUsage(Usage)
              .setSharingMode(vk::SharingMode::eExclusive);

    {
        auto Result = Context->pDevice->LogicalDevice.createBuffer(CreateInfo, Context->Allocator);
        if (!VulkanUtils::ResultIsSuccess(Result.result))
        {
            MlokError("Error creating Vulkan Buffer: %s", VulkanUtils::VulkanResultString(Result.result, true).c_str());
            return false;
        }
        Handle = Result.value;
    }

    vk::MemoryRequirements Requirements = Context->pDevice->LogicalDevice.getBufferMemoryRequirements(Handle);
    MemoryIndex = Context->FindMemoryIndex(Requirements.memoryTypeBits, MemoryPropertyFlags);
    if (MemoryIndex == -1)
    {
        MlokError("Unable to create Vulkan Buffer because the required memory type index was not found");
        return false;
    }

    vk::MemoryAllocateInfo AllocateInfo {};
    AllocateInfo.setAllocationSize(Requirements.size)
                .setMemoryTypeIndex(static_cast<uint32_t>(MemoryIndex));
    
    {
        auto Result = Context->pDevice->LogicalDevice.allocateMemory(AllocateInfo, Context->Allocator);
        if (!VulkanUtils::ResultIsSuccess(Result.result))
        {
            MlokError("Unable to create vulkan buffer because the required memory allocation failed: %s", VulkanUtils::VulkanResultString(Result.result, true).c_str());
            return false;
        }
        Memory = Result.value;
    }

    if (bBindOnCreate)
    {
        Bind(0);
    }

    return true;
}

void VulkanBuffer::Destroy()
{
    if (Memory)
    {
        Context->pDevice->LogicalDevice.freeMemory(Memory, Context->Allocator);
        Memory = nullptr;
    }
    if (Handle)
    {
        Context->pDevice->LogicalDevice.destroyBuffer(Handle, Context->Allocator);
        Handle = nullptr;
    }
    TotalSize = 0;
    bIsLocked = false;
}

bool VulkanBuffer::Resize(size_t NewSize, vk::Queue Queue, vk::CommandPool Pool)
{
    vk::BufferCreateInfo CreateInfo {};
    CreateInfo.setSize(NewSize)
              .setUsage(Usage)
              .setSharingMode(vk::SharingMode::eExclusive);

    vk::Buffer NewBuffer;
    
    {
        auto Result = Context->pDevice->LogicalDevice.createBuffer(CreateInfo, Context->Allocator);
        if (!VulkanUtils::ResultIsSuccess(Result.result))
        {
            MlokError("Error resizing Vulkan Buffer on new buffer creation: %s", VulkanUtils::VulkanResultString(Result.result, true).c_str());
            return false;
        }
        NewBuffer = Result.value;
    }

    vk::MemoryRequirements Requirements = Context->pDevice->LogicalDevice.getBufferMemoryRequirements(NewBuffer);

    vk::MemoryAllocateInfo AllocateInfo {};
    AllocateInfo.setAllocationSize(Requirements.size)
                .setMemoryTypeIndex(static_cast<uint32_t>(MemoryIndex));

    vk::DeviceMemory NewMemory;

    {
        auto Result = Context->pDevice->LogicalDevice.allocateMemory(AllocateInfo, Context->Allocator);
        if (!VulkanUtils::ResultIsSuccess(Result.result))
        {
            MlokError("Unable to resize vulkan buffer because the required memory allocation failed: %s", VulkanUtils::VulkanResultString(Result.result, true).c_str());
            return false;
        }
        NewMemory = Result.value;
    }

    {
        auto Result = Context->pDevice->LogicalDevice.bindBufferMemory(NewBuffer, NewMemory, 0);
        if (!VulkanUtils::ResultIsSuccess(Result))
        {
            MlokError("Unable to resize vulkan buffer because memory binding failed: %s", VulkanUtils::VulkanResultString(Result, true).c_str());
            return false;
        }
    }

    VulkanBuffer::Copy(Context, Pool, nullptr, Queue, Handle, 0, NewBuffer, 0, TotalSize);

    auto WaitResult = Context->pDevice->LogicalDevice.waitIdle();

    if (Memory)
    {
        Context->pDevice->LogicalDevice.freeMemory(Memory, Context->Allocator);
    }
    if (Handle)
    {
        Context->pDevice->LogicalDevice.destroyBuffer(Handle, Context->Allocator);
    }

    TotalSize = NewSize;
    Memory = NewMemory;
    Handle = NewBuffer;

    return true;
}

void VulkanBuffer::Bind(size_t Offset)
{
    {
        auto Result = Context->pDevice->LogicalDevice.bindBufferMemory(Handle, Memory, Offset);
        if (!VulkanUtils::ResultIsSuccess(Result))
        {
            MlokError("Error binding Vulkan Buffer: %s", VulkanUtils::VulkanResultString(Result, true).c_str());
        }
    }
}

void* VulkanBuffer::LockMemory(size_t Offset, size_t Size, vk::MemoryMapFlags Flags)
{
    void* pData = nullptr;
    {
        auto Result = Context->pDevice->LogicalDevice.mapMemory(Memory, static_cast<vk::DeviceSize>(Offset), static_cast<vk::DeviceSize>(Size), Flags);
        if (!VulkanUtils::ResultIsSuccess(Result.result))
        {
            MlokError("Error locking memory by Vulkan Buffer: %s", VulkanUtils::VulkanResultString(Result.result, true).c_str());
            return nullptr;
        }
        pData = Result.value;
    }

    return pData;
}

void VulkanBuffer::UnlockMemory()
{
    Context->pDevice->LogicalDevice.unmapMemory(Memory);
}

void VulkanBuffer::LoadData(size_t Offset, size_t Size, vk::MemoryMapFlags Flags, const void* Data)
{
    void* pData = nullptr;
    {
        auto Result = Context->pDevice->LogicalDevice.mapMemory(Memory, Offset, Size, Flags);
        if (!VulkanUtils::ResultIsSuccess(Result.result))
        {
            MlokError("Error loading memory by Vulkan Buffer: %s", VulkanUtils::VulkanResultString(Result.result, true).c_str());
            return;
        }
        pData = Result.value;
    }
    Platform::Get()->PlatformCopyMemory(pData, Data, Size); // TODO: create some static wrapper in MemorySystem
    
    Context->pDevice->LogicalDevice.unmapMemory(Memory);
}

void VulkanBuffer::Copy(VulkanContext* Context,
                        vk::CommandPool Pool,
                        vk::Fence Fence,
                        vk::Queue Queue,
                        vk::Buffer Source,
                        size_t SourceOffset,
                        vk::Buffer Dest,
                        size_t DestOffset,
                        size_t Size)
{
    auto WaitResult = Queue.waitIdle();
    VulkanCommandBuffer TmpCommandBuffer {};
    TmpCommandBuffer.AllocateAndBeginSingleUse(Context, Pool);

    vk::BufferCopy CopyRegion;
    CopyRegion.setSrcOffset(SourceOffset)
              .setDstOffset(DestOffset)
              .setSize(Size);

    TmpCommandBuffer.Get()->copyBuffer(Source, Dest, 1, &CopyRegion);

    TmpCommandBuffer.EndSingleUse(Pool, Queue);
}