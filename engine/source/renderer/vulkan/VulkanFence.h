#pragma once

#include "VulkanTypes.inl"

class VulkanContext;

class VulkanFence
{
    public:
        VulkanFence() = default;
        VulkanFence(VulkanContext* Context, bool bCreateSignaled);
        VulkanFence(const VulkanFence&) = delete;
        VulkanFence(VulkanFence&&) = default;
        ~VulkanFence();

        VulkanFence& operator=(const VulkanFence&) = delete;

        vk::Fence* Get() { return &Handle; }

        void Create(VulkanContext* Context, bool bCreateSignaled);
        void Destroy();

        bool Wait(uint64_t TimeoutNs);
        void Reset();

    private:
        VulkanContext* Context;

        vk::Fence Handle;
        bool bIsSignaled;
};