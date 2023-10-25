#pragma once

#include "VulkanTypes.inl"

class VulkanContext;
class VulkanCommandBuffer;

enum class VulkanRenderPassState
{
    eReady,
    eRecording,
    eInRenderPass,
    eRecordingEnded,
    eSubmitted,
    eNotAllocated
};

class VulkanRenderPass
{
    public:
        VulkanRenderPass() = delete;
        VulkanRenderPass(VulkanContext* Context,
                         float X, float Y, float W, float H,
                         float R, float G, float B, float A, // Clear color
                         float Depth, uint32_t Stencil);
        VulkanRenderPass(const VulkanRenderPass&) = delete;
        VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;
        ~VulkanRenderPass();

        MINLINE vk::RenderPass* Get() { return &Handle; }

        void Create(VulkanContext* Context);

        void Destroy();

        void Begin(VulkanCommandBuffer* CommandBuffer,
                   vk::Framebuffer Framebuffer);

        void End(VulkanCommandBuffer* CommandBuffer);
    
    private:
        VulkanContext* Context; // Cached pointer to backend context

        vk::RenderPass Handle;
        float X, Y, W, H;
        float R, G, B, A; // Clear color

        float Depth;
        uint32_t Stencil;

        VulkanRenderPassState State;
};