#include "VulkanRenderPass.h"

#include "VulkanContext.h"
#include "VulkanCommandBuffer.h"

VulkanRenderPass::VulkanRenderPass(VulkanContext* inContext,
                                   float inX, float inY, float inW, float inH,
                                   float inR, float inG, float inB, float inA, // Clear color
                                   float inDepth, uint32_t inStencil)
    : Context { inContext }
    , X {inX}, Y {inY}, W {inW}, H {inH}
    , R {inR}, G {inG}, B {inB}, A {inA}
    , Depth {inDepth}, Stencil {inStencil}
{
    Create(inContext);
}

VulkanRenderPass::~VulkanRenderPass()
{
    Destroy();
}

void VulkanRenderPass::Create(VulkanContext* inContext)
{
    if (Context != inContext)
    {
        Context = inContext;
    }

    std::vector<vk::AttachmentDescription> AttachmentDescriptions;

    vk::AttachmentDescription ColorAttachment {};
    ColorAttachment.setFormat(Context->pSwapchain->GetImageFormat().format)
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setLoadOp(vk::AttachmentLoadOp::eClear)
                   .setStoreOp(vk::AttachmentStoreOp::eStore)
                   .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    AttachmentDescriptions.push_back(ColorAttachment);

    vk::AttachmentDescription DepthAttachment {};
    DepthAttachment.setFormat(Context->pDevice->GetDepthFormat())
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setLoadOp(vk::AttachmentLoadOp::eClear)
                   .setStoreOp(vk::AttachmentStoreOp::eStore)
                   .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    AttachmentDescriptions.push_back(DepthAttachment);

    vk::AttachmentReference ColorAttachmentReference {};
    ColorAttachmentReference.setAttachment(0)
                            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentReference DepthAttachmentReference {};
    DepthAttachmentReference.setAttachment(1)
                            .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription Subpass;
    Subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
           .setColorAttachmentCount(1)
           .setPColorAttachments(&ColorAttachmentReference)
           .setPDepthStencilAttachment(&DepthAttachmentReference);

    vk::SubpassDependency SubpassDependency {};
    SubpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
                     .setDstSubpass(0u)
                     .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                     .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                     .setSrcAccessMask(vk::AccessFlagBits::eNone)
                     .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
    
    vk::RenderPassCreateInfo CreateInfo {};
    CreateInfo.setAttachments(AttachmentDescriptions)
              .setSubpassCount(1)
              .setPSubpasses(&Subpass)
              .setDependencyCount(1)
              .setPDependencies(&SubpassDependency);

    Handle = Context->pDevice->LogicalDevice.createRenderPass(CreateInfo, Context->Allocator);
}

void VulkanRenderPass::Destroy()
{
    if (Handle)
    {
        Context->pDevice->LogicalDevice.destroyRenderPass(Handle, Context->Allocator);
        Handle = nullptr;
    }

    Context = nullptr;
}

void VulkanRenderPass::Begin(VulkanCommandBuffer* CommandBuffer,
                             vk::Framebuffer Framebuffer)
{
    const vk::Rect2D RenderArea { { static_cast<int32_t>(X), static_cast<int32_t>(Y) }, 
                                  { static_cast<uint32_t>(W), static_cast<uint32_t>(H) } };
    
    std::vector<vk::ClearValue> ClearValues;
    ClearValues.push_back(vk::ClearValue().setColor( { R, G, B, A } ));
    ClearValues.push_back(vk::ClearValue().setDepthStencil( { Depth, Stencil } ));

    vk::RenderPassBeginInfo BeginInfo {};
    BeginInfo.setRenderPass(Handle)
             .setFramebuffer(Framebuffer)
             .setRenderArea(RenderArea)
             .setClearValues(ClearValues);
    
    CommandBuffer->Get()->beginRenderPass(BeginInfo, vk::SubpassContents::eInline);
    CommandBuffer->UpdateInRenderPass();
}

void VulkanRenderPass::End(VulkanCommandBuffer* CommandBuffer)
{
    CommandBuffer->Get()->endRenderPass();
    CommandBuffer->UpdateRecording();
}