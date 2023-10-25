#include "VulkanImage.h"

#include "VulkanContext.h"

#include "core/Logger.h"

VulkanImage::VulkanImage()
{

}

VulkanImage::VulkanImage(VulkanContext* inContext,
                         vk::ImageType ImageType,
                         uint32_t inWidth,
                         uint32_t inHeight,
                         vk::Format inFormat,
                         vk::ImageTiling Tiling,
                         vk::ImageUsageFlags UsageFlags,
                         vk::MemoryPropertyFlags MemoryPropertyFlags,
                         bool bShouldCreateView,
                         vk::ImageAspectFlags ImageAspectFlags)
    : Context { inContext }
    , Width { inWidth }
    , Height { inHeight }
    , Format { inFormat }
    , AspectFlags {ImageAspectFlags}
{
    Create(inContext, ImageType, Tiling, UsageFlags, MemoryPropertyFlags, bShouldCreateView);
}

VulkanImage::~VulkanImage()
{
    Destroy();
}

void VulkanImage::Create(VulkanContext* inContext,
                         vk::ImageType ImageType,
                         vk::ImageTiling Tiling,
                         vk::ImageUsageFlags UsageFlags,
                         vk::MemoryPropertyFlags MemoryPropertyFlags,
                         bool bShouldCreateView)
{
    vk::ImageCreateInfo CreateInfo {};
    CreateInfo.setImageType(vk::ImageType::e2D)
              .setExtent({ Width, Height, 1 })
              .setMipLevels(4)
              .setArrayLayers(1)
              .setFormat(Format)
              .setTiling(Tiling)
              .setInitialLayout(vk::ImageLayout::eUndefined)
              .setUsage(UsageFlags)
              .setSamples(vk::SampleCountFlagBits::e1)
              .setSharingMode(vk::SharingMode::eExclusive);

    Handle = Context->pDevice->LogicalDevice.createImage(CreateInfo, Context->Allocator);

    vk::MemoryRequirements MemoryRequirements;
    MemoryRequirements = Context->pDevice->LogicalDevice.getImageMemoryRequirements(Handle);

    int32_t MemoryType = Context->FindMemoryIndex(MemoryRequirements.memoryTypeBits, MemoryPropertyFlags);
    if (MemoryType == -1)
    {
        MlokError("Required memory type for image not found");
        return;
    }

    vk::MemoryAllocateInfo MemoryAllocateInfo {};
    MemoryAllocateInfo.setAllocationSize(MemoryRequirements.size)
                      .setMemoryTypeIndex(MemoryType);
    Memory = Context->pDevice->LogicalDevice.allocateMemory(MemoryAllocateInfo, Context->Allocator);
    Context->pDevice->LogicalDevice.bindImageMemory(Handle, Memory, 0u);

    if (bShouldCreateView)
    {
        View = nullptr;
        CreateImageView();
    }
}

void VulkanImage::CreateImageView()
{
    vk::ImageSubresourceRange SubresourceRange {};
    SubresourceRange.setAspectMask(AspectFlags)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

    vk::ImageViewCreateInfo CreateInfo {};
    CreateInfo.setImage(Handle)
              .setViewType(vk::ImageViewType::e2D)
              .setFormat(Format)
              .setSubresourceRange(SubresourceRange);

    View = Context->pDevice->LogicalDevice.createImageView(CreateInfo, Context->Allocator);
}

void VulkanImage::Destroy()
{
    if (View)
    {
        Context->pDevice->LogicalDevice.destroyImageView(View);
        View = nullptr;
    }
    if (Memory)
    {
        Context->pDevice->LogicalDevice.freeMemory(Memory, Context->Allocator);
        Memory = nullptr;
    }
    if (Handle)
    {
        Context->pDevice->LogicalDevice.destroyImage(Handle, Context->Allocator);
        Handle = nullptr;
    }

    Context = nullptr;
}   