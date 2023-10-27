#pragma once

#include "VulkanTypes.inl"

class VulkanContext;

enum class VulkanCommandBufferState
{
    eReady,
    eRecording,
    eInRenderPass,
    eRecordingEnded,
    eSubmitted,
    eNotAllocated
};

class VulkanCommandBuffer
{
    public:
        VulkanCommandBuffer() = default;
        VulkanCommandBuffer(VulkanContext* Context,
                            vk::CommandPool CommandPool,
                            bool bIsPrimary,
                            bool bIsSingleUse = false);
        VulkanCommandBuffer(const VulkanCommandBuffer&) = default;
        VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
        ~VulkanCommandBuffer();

        MINLINE vk::CommandBuffer* Get() { return &Handle; }

        void Allocate(VulkanContext* Context,
                      vk::CommandPool CommandPool,
                      bool bIsPrimary);
        
        void Free();

        void Begin(bool bIsSingleUse,
                   bool bIsRenderPassContinue,
                   bool bIsSimulteneousUse);
        void End();
        
        void UpdateRecording();
        void UpdateSubmitted();
        void UpdateInRenderPass();
        void Reset();
        
        void AllocateAndBeginSingleUse(VulkanContext* Context, vk::CommandPool CommandPool);
        void EndSingleUse(vk::CommandPool CommandPool, vk::Queue Queue);

    private:
        VulkanContext* Context; // Cached pointer to backend context
        vk::CommandPool OwningCommandPool;

        vk::CommandBuffer Handle;
        VulkanCommandBufferState State;
};