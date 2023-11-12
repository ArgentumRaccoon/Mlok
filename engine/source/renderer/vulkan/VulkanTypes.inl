#pragma once

#include "Defines.h"

#include "core/MlokMemory.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#define OBJECT_SHADER_STAGE_COUNT 2

typedef enum VulkanEventCode
{

} VulkanEventCode;

typedef struct VulkanSwapchainSupportInfo
{
    vk::SurfaceCapabilitiesKHR SurfaceCapabilities;
    std::vector<vk::SurfaceFormatKHR> Formats; // TODO: use allocator with tag
    std::vector<vk::PresentModeKHR> PresentModes;// TODO: use allocator with tag
} VulkanSwapchainSupportInfo;