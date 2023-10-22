#pragma once

#include "VulkanTypes.inl"

#include "VulkanDevice.h"

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

        uint32_t FramebufferWidth;
        uint32_t FramebufferHeight;
};