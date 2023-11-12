#include "VulkanFence.h"

#include "VulkanContext.h"

#include "core/Logger.h"

VulkanFence::VulkanFence(VulkanContext* inContext, bool bCreateSignaled)
{
    Create(inContext, bCreateSignaled);
}

VulkanFence::~VulkanFence()
{
    Destroy();
}

void VulkanFence::Create(VulkanContext* inContext, bool bCreateSignaled)
{
    Context = inContext;
    bIsSignaled = bCreateSignaled;

    vk::FenceCreateInfo CreateInfo {};
    if (bIsSignaled)
    {
        CreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    }
    Handle = Context->pDevice->LogicalDevice.createFence(CreateInfo, Context->Allocator).value;
}

void VulkanFence::Destroy()
{
    if (Handle)
    {
        Context->pDevice->LogicalDevice.destroyFence(Handle, Context->Allocator);
        Handle = nullptr;
    }
    bIsSignaled = false;
}

bool VulkanFence::Wait(uint64_t TimeoutNs)
{
    if (bIsSignaled)
    {
        return true;
    }

    std::vector<vk::Fence> Fences { Handle };
    switch(Context->pDevice->LogicalDevice.waitForFences(Fences, true, TimeoutNs))
    {
        case vk::Result::eSuccess:
            bIsSignaled = true;
            return true;
        case vk::Result::eTimeout:
            MlokWarning("VulkanFence Wait: timed out");
            break;
        case vk::Result::eErrorDeviceLost:
            MlokError("Error VulkanFence Wait: Device Lost");
            break;
        case vk::Result::eErrorOutOfHostMemory:
            MlokError("Error VulkanFence Wait: Out of Host Memory");
            break;
        case vk::Result::eErrorOutOfDeviceMemory:
            MlokError("Error VulkanFence Wait: Out of Device Memory");
            break;
        default:
            MlokError("Error VulkanFence Wait: Unknown");
            break;                
    }
    return false;
}

void VulkanFence::Reset()
{
    if (bIsSignaled)
    {
        std::vector<vk::Fence> Fences { Handle };
        Context->pDevice->LogicalDevice.resetFences(Fences);
        bIsSignaled = false;
    }
}