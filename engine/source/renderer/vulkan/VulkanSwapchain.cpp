#include "VulkanSwapchain.h"

#include "VulkanContext.h"
#include "VulkanRenderPass.h"

#include "core/Logger.h"
#include "core/MlokUtils.h"

#include <algorithm>

VulkanSwapchain::VulkanSwapchain(VulkanContext* inContext, uint32_t Width, uint32_t Height)
{
    Create(inContext, Width, Height);
}

VulkanSwapchain::~VulkanSwapchain()
{
    Destroy();
}

void VulkanSwapchain::Create(VulkanContext* inContext, uint32_t Width, uint32_t Height)
{
    CreateInternal(inContext, Width, Height);
}

void VulkanSwapchain::Recreate(uint32_t Width, uint32_t Height)
{
    VulkanContext* SavedContext = Context; // Save before destroying to pass later
    DestroyInternal();
    CreateInternal(SavedContext, Width, Height);
}

void VulkanSwapchain::Destroy()
{
    DestroyInternal();
}

bool VulkanSwapchain::AcquireNextImageIndex(uint64_t TimeoutNs,
                                            vk::Semaphore ImageAvailableSemaphore,
                                            vk::Fence Fence,
                                            uint32_t* OutImageIndex)
{
    auto Result = Context->pDevice->LogicalDevice.acquireNextImageKHR(Handle, TimeoutNs, ImageAvailableSemaphore, Fence);

    if (Result.result == vk::Result::eErrorOutOfDateKHR)
    {
        Recreate(Context->FramebufferWidth, Context->FramebufferHeight);
        return false;
    }
    else if (Result.result != vk::Result::eSuccess && Result.result != vk::Result::eSuboptimalKHR)
    {
        MlokFatal("Failed to acquire Vulkan Swapchain image");
        return false;
    }

    *OutImageIndex = Result.value;
    return true;
}

void VulkanSwapchain::Present(vk::Queue GraphicsQueue,
                              vk::Queue PresentQueue,
                              vk::Semaphore RenderCompleteSemaphore,
                              uint32_t PresentImageIndex)
{
    vk::PresentInfoKHR PresentInfo {};
    PresentInfo.setWaitSemaphoreCount(1)
               .setPWaitSemaphores(&RenderCompleteSemaphore)
               .setSwapchainCount(1)
               .setPSwapchains(&Handle)
               .setPImageIndices(&PresentImageIndex);

    vk::Result Result = PresentQueue.presentKHR(PresentInfo);
    if (Result == vk::Result::eErrorOutOfDateKHR || Result == vk::Result::eSuboptimalKHR)
    {
        Recreate(Context->FramebufferWidth, Context->FramebufferHeight);
    }

    Context->CurrentFrame = (Context->CurrentFrame + 1) % MaxFramesInFlight;
}

void VulkanSwapchain::RegenerateFramebuffers(VulkanRenderPass* RenderPass)
{
    if (Framebuffers.size() != ImageCount)
    {
        Framebuffers.resize(ImageCount);
    }

    for (size_t i = 0; i < ImageCount; ++i)
    {
        uint32_t AttachmentCount = 2; // per framebuffer
        std::vector<vk::ImageView> Attachments = { Views[i], DepthAttachment->GetView() };
        Framebuffers[i].Create(Context, RenderPass, Context->FramebufferWidth, Context->FramebufferHeight, Attachments);
    }
}

void VulkanSwapchain::DestroyFramebuffers()
{
    for (auto& Framebuffer : Framebuffers)
    {
        Framebuffer.Destroy();
    }
}

void VulkanSwapchain::CreateInternal(VulkanContext* inContext, uint32_t Width, uint32_t Height)
{
    if (Context != inContext)
    {
        Context = inContext;
    }

    vk::Extent2D Extent { Width, Height };

    auto& FormatsList = Context->pDevice->GetSwapchainSupport().Formats;
    ImageFormat = FormatsList[0]; // by default
    
    auto FoundFormat = std::find(FormatsList.begin(),
                                 FormatsList.end(),
                                 vk::SurfaceFormatKHR { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear });

    if (FoundFormat != FormatsList.end())
    {
        ImageFormat = *FoundFormat;
    }

    vk::PresentModeKHR PresentMode = vk::PresentModeKHR::eFifo; // by default
    auto& PresentModesList = Context->pDevice->GetSwapchainSupport().PresentModes;
    
    auto FoundPresentMode = std::find(PresentModesList.begin(),
                                      PresentModesList.end(),
                                      vk::PresentModeKHR::eMailbox);
    
    if (FoundPresentMode != PresentModesList.end())
    {
        PresentMode = *FoundPresentMode;
    }

    
    Context->pDevice->QuerySwapchainSupport(Context->pDevice->PhysicalDevice, Context->Surface);

    if (Context->pDevice->GetSwapchainSupport().SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        Extent = Context->pDevice->GetSwapchainSupport().SurfaceCapabilities.currentExtent;
    }

    vk::Extent2D Min = Context->pDevice->GetSwapchainSupport().SurfaceCapabilities.minImageExtent;
    vk::Extent2D Max = Context->pDevice->GetSwapchainSupport().SurfaceCapabilities.maxImageExtent;
    Width  = MlokUtils::Clamp(Width, Min.width, Max.width);
    Height = MlokUtils::Clamp(Height, Min.height, Max.height);

    ImageCount = Context->pDevice->GetSwapchainSupport().SurfaceCapabilities.minImageCount + 1;
    const uint32_t SurfaceMaxImageCount = Context->pDevice->GetSwapchainSupport().SurfaceCapabilities.maxImageCount;
    if (SurfaceMaxImageCount > 0 && ImageCount > SurfaceMaxImageCount)
    {
        ImageCount = SurfaceMaxImageCount;
    }
    MaxFramesInFlight = ImageCount - 1;

    vk::SwapchainCreateInfoKHR CreateInfo {};
    CreateInfo.setSurface(Context->Surface)
              .setMinImageCount(ImageCount)
              .setImageFormat(ImageFormat.format)
              .setImageColorSpace(ImageFormat.colorSpace)
              .setImageExtent(Extent)
              .setImageArrayLayers(1)
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
              .setPreTransform(Context->pDevice->GetSwapchainSupport().SurfaceCapabilities.currentTransform)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setPresentMode(PresentMode)
              .setClipped(VK_TRUE);

    if (Context->pDevice->GetGraphicsQueueIndex() != Context->pDevice->GetPresentQueueIndex())
    {
        std::vector<uint32_t> QueueFamilyIndices = { (uint32_t)Context->pDevice->GetGraphicsQueueIndex(),
                                                     (uint32_t)Context->pDevice->GetPresentQueueIndex() };
        CreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
                  .setQueueFamilyIndices(QueueFamilyIndices);
    }
    else
    {
        CreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    Handle = Context->pDevice->LogicalDevice.createSwapchainKHR(CreateInfo, Context->Allocator).value;

    Context->CurrentFrame = 0;
    ImageCount = 0;

    Images = Context->pDevice->LogicalDevice.getSwapchainImagesKHR(Handle).value;
    ImageCount = Images.size();
    
    if (Views.empty())
    {
        Views.resize(Images.size());
    }

    for (size_t i = 0; i < Images.size(); ++i)
    {
        vk::ImageSubresourceRange SubresourceRange {};
        SubresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setBaseMipLevel(0)
                        .setLevelCount(1)
                        .setBaseArrayLayer(0)
                        .setLayerCount(1);

                        
        vk::ImageViewCreateInfo ViewInfo {};
        ViewInfo.setImage(Images[i])
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(ImageFormat.format)
                .setSubresourceRange(SubresourceRange);

        Views[i] = Context->pDevice->LogicalDevice.createImageView(ViewInfo, Context->Allocator).value;
    }

    if (!Context->pDevice->DetectDepthFormat())
    {
        Context->pDevice->SetDepthFormat(vk::Format::eUndefined);
        MlokFatal("Failed to find supported Depth format");
    }

    DepthAttachment = std::make_unique<VulkanImage>(Context,
                                                    vk::ImageType::e2D,
                                                    Extent.width,
                                                    Extent.height,
                                                    Context->pDevice->GetDepthFormat(),
                                                    vk::ImageTiling::eOptimal,
                                                    vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                                    vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                    true,
                                                    vk::ImageAspectFlagBits::eDepth);
}

void VulkanSwapchain::DestroyInternal()
{
    Context->pDevice->LogicalDevice.waitIdle();
    
    DepthAttachment->Destroy();

    for (auto& View : Views)
    {
        Context->pDevice->LogicalDevice.destroyImageView(View, Context->Allocator);
        View = nullptr;
    }

    Context->pDevice->LogicalDevice.destroySwapchainKHR(Handle, Context->Allocator);
    Handle = nullptr;
}