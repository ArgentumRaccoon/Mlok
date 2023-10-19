#pragma once

#include "VulkanTypes.inl"

class VulkanDevice
{
    public:
        vk::PhysicalDevice PhysicalDevice;
        vk::Device LogicalDevice;
};