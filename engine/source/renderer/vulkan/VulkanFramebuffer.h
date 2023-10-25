#pragma once

#include "VulkanTypes.inl"

class VulkanContext;
class VulkanRenderPass;

class VulkanFramebuffer
{
    public:
        VulkanFramebuffer() = delete;
        VulkanFramebuffer(VulkanContext* Context,
                          VulkanRenderPass* RenderPass,
                          uint32_t Width,
                          uint32_t Height,
                          std::vector<vk::ImageView>& Attachments);
        VulkanFramebuffer(const VulkanFramebuffer&) = delete;
        VulkanFramebuffer& operator=(const VulkanFramebuffer&) = delete;
        ~VulkanFramebuffer();

        MINLINE vk::Framebuffer* Get() { return &Handle; }

        void Create(VulkanContext* Context,
                    uint32_t Width,
                    uint32_t Height);

        void Destroy();

    private:
        VulkanContext* Context; // Cached pointer to backend context

        vk::Framebuffer Handle;
        std::vector<vk::ImageView> Attachments; // TODO: use allocator with tag
        VulkanRenderPass* RenderPass;
};