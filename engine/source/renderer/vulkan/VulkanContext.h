#pragma once

#include "VulkanTypes.inl"

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include <memory>

class VulkanContext
{
    public:
        std::unique_ptr<vk::Instance> pInstance;
        vk::AllocationCallbacks* Allocator;
        vk::SurfaceKHR Surface;
#ifndef NDEBUG
        vk::DebugUtilsMessengerEXT DebugMessenger;
#endif
        
        std::unique_ptr<VulkanDevice> pDevice;
        std::unique_ptr<VulkanSwapchain> pSwapchain;

        uint32_t FramebufferWidth;
        uint32_t FramebufferHeight;

        uint32_t CurrentFrame;

        virtual int32_t FindMemoryIndex(uint32_t TypeFilter, vk::MemoryPropertyFlags PropertyFlags);
};