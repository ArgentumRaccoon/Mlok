#pragma once

#include "VulkanTypes.inl"

#include "VulkanImage.h"

#include <memory>

class VulkanContext;
class VulkanFramebuffer;

class VulkanSwapchain
{
    public:
        VulkanSwapchain() = delete;
        VulkanSwapchain(VulkanContext* Context, uint32_t Width, uint32_t Height);
        VulkanSwapchain(const VulkanSwapchain&) = delete;
        VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
        ~VulkanSwapchain();

        MINLINE vk::SwapchainKHR* Get() { return &Handle; }

        void Create(VulkanContext* Context, uint32_t Width, uint32_t Height);
        void Recreate(uint32_t Width, uint32_t Height);
        void Destroy();

        bool AcquireNextImageIndex(uint64_t TimeoutNs,
                                   vk::Semaphore ImageAvailableSemaphore,
                                   vk::Fence Fence,
                                   uint32_t* OutImageIndex);
        void Present(vk::Queue GraphicsQueue,
                     vk::Queue PresentQueue,
                     vk::Semaphore RenderCompleteSemaphore,
                     uint32_t PresentImageIndex);

        const vk::SurfaceFormatKHR& GetImageFormat() const { return ImageFormat; }

    private:
        VulkanContext* Context; // Cached pointer to backend context

        vk::SwapchainKHR Handle;

        vk::SurfaceFormatKHR ImageFormat;
        uint8_t MaxFramesInFlight;
        uint32_t ImageCount;
        std::vector<vk::Image> Images;
        std::vector<vk::ImageView> Views;

        std::unique_ptr<VulkanImage> DepthAttachment;
        VulkanFramebuffer* Framebuffers;

        void CreateInternal(VulkanContext* inContext, uint32_t Width, uint32_t Height);
        void DestroyInternal();
};