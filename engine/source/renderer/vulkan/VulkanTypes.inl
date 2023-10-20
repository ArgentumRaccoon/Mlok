#pragma once

#include "Defines.h"

#include <vulkan/vulkan.hpp>

#include "VulkanDevice.h"

class VulkanContext
{
    public:
        vk::Instance Instance;
        vk::AllocationCallbacks* Allocator;
        vk::SurfaceKHR Surface;
#ifndef NDEBUG
        vk::DebugUtilsMessengerEXT DebugMessanger;
#endif
        
        VulkanDevice Device;

        uint32_t FramebufferWidth;
        uint32_t FramebufferHeight;
};