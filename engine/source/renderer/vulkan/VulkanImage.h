#pragma once

#include "VulkanTypes.inl"

class VulkanContext;

class VulkanImage
{
    public:
        VulkanImage();
        VulkanImage(VulkanContext* Context,
                    vk::ImageType ImageType,
                    uint32_t Width,
                    uint32_t Height,
                    vk::Format inFormat,
                    vk::ImageTiling Tiling,
                    vk::ImageUsageFlags UsageFlags,
                    vk::MemoryPropertyFlags MemoryPropertyFlags,
                    bool bShouldCreateView,
                    vk::ImageAspectFlags ImageAspectFlags);
        ~VulkanImage();

        MINLINE vk::Image* Get() { return &Handle; }

        void Create(VulkanContext* Context,
                    vk::ImageType ImageType,
                    vk::ImageTiling Tiling,
                    vk::ImageUsageFlags UsageFlags,
                    vk::MemoryPropertyFlags MemoryPropertyFlags,
                    bool bShouldCreateView);

        void CreateImageView();
        
        void Destroy();

        vk::ImageView& GetView() { return View; }
    
    private:
        VulkanContext* Context; // Cached pointer to backend context

        vk::Image Handle;
        vk::DeviceMemory Memory;
        vk::ImageView View;
        uint32_t Width;
        uint32_t Height;
        vk::Format Format;
        vk::ImageAspectFlags AspectFlags;
};