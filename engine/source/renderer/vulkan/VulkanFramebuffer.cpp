#include "VulkanFramebuffer.h"

#include "VulkanContext.h"
#include "VulkanRenderPass.h"

VulkanFramebuffer::VulkanFramebuffer(VulkanContext* inContext,
                                     VulkanRenderPass* inRenderPass,
                                     uint32_t Width,
                                     uint32_t Height,
                                     std::vector<vk::ImageView>& inAttachments)
{
    Create(inContext, RenderPass, Width, Height, Attachments);
}

VulkanFramebuffer::~VulkanFramebuffer()
{
    Destroy();
}

void VulkanFramebuffer::Create(VulkanContext* inContext,
                               VulkanRenderPass* inRenderPass,
                               uint32_t Width,
                               uint32_t Height,
                               std::vector<vk::ImageView>& inAttachments)
{
    Context = inContext;
    Attachments = inAttachments;
    RenderPass = inRenderPass;

    vk::FramebufferCreateInfo CreateInfo {};
    CreateInfo.setRenderPass(*RenderPass->Get())
              .setAttachments(Attachments)
              .setWidth(Width)
              .setHeight(Height)
              .setLayers(1);

    Handle = Context->pDevice->LogicalDevice.createFramebuffer(CreateInfo, Context->Allocator);
}

void VulkanFramebuffer::Recreate(VulkanContext* inContext,
                                 VulkanRenderPass* inRenderPass,
                                 uint32_t Width,
                                 uint32_t Height,
                                 std::vector<vk::ImageView>& inAttachments)
{
    VulkanFramebuffer(inContext,
                      inRenderPass,
                      Width, Height,
                      inAttachments);
}

void VulkanFramebuffer::Destroy()
{
    if (Handle)
    {
        Context->pDevice->LogicalDevice.destroyFramebuffer(Handle, Context->Allocator);

        Attachments.clear();

        RenderPass = nullptr;
        Handle = nullptr;
    }

    Context = nullptr;
}