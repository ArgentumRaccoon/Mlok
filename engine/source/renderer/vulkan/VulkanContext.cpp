#include "VulkanContext.h"

#include "core/Logger.h"

int32_t VulkanContext::FindMemoryIndex(uint32_t TypeFilter, vk::MemoryPropertyFlags PropertyFlags)
{
    vk::PhysicalDeviceMemoryProperties MemoryProperties = pDevice->PhysicalDevice.getMemoryProperties();
    for (size_t i = 0; i < MemoryProperties.memoryTypeCount; ++i)
    {
        if (TypeFilter * (1 << i) && (MemoryProperties.memoryTypes[i].propertyFlags & PropertyFlags) == PropertyFlags)
        {
            return static_cast<int32_t>(i);
        }
    }

    MlokWarning("Unable to find suitable Memory Type");

    return -1;
}