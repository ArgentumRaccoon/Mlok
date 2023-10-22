#pragma once

#include "VulkanTypes.inl"

typedef struct VulkanPhysicalDeviceRequirements
{
    bool bGraphics;
    bool bPresent;
    bool bCompute;
    bool bTransfer;

    bool bSamplerAnisotropy;
    bool bDiscreteGPU;

    std::vector<std::string> DeviceExtensionNames;
} VulkanPhysicalDeviceRequirements;

typedef struct VulkanPhysicalDeviceQueueFamilyInfo
{
    uint32_t GraphicsFamilyIndex;
    uint32_t PresentFamilyIndex;
    uint32_t TransferFamilyIndex;
    uint32_t ComputeFamilyIndex;
} VulkanPhysicalDeviceQueueFamilyInfo;

class VulkanContext;

class VulkanDevice
{
    public:
        VulkanDevice() = delete;
        VulkanDevice(VulkanContext* Context);
        VulkanDevice(const VulkanDevice& Other) = delete;        
        VulkanDevice& operator=(const VulkanDevice& Other) = delete;
        ~VulkanDevice();

        bool Create(VulkanContext* Context);
        void Destroy();

        void QuerySwapchainSupport(const vk::PhysicalDevice& inPhysicalDevice,
                                   VkSurfaceKHR Surface,
                                   VulkanSwapchainSupportInfo* OutSupportInfo) const;

        bool PhysicalDeviceMeetsRequirements(const vk::PhysicalDevice& PhysicalDeviceToCheck,
                                             vk::SurfaceKHR Surface,
                                             const vk::PhysicalDeviceProperties* Properties,
                                             const vk::PhysicalDeviceFeatures* Features,
                                             const VulkanPhysicalDeviceRequirements* Requirements,
                                             VulkanPhysicalDeviceQueueFamilyInfo* OutQueueFamilyInfo,
                                             VulkanSwapchainSupportInfo* OutSwapchainSupportInfo) const;

        vk::PhysicalDevice PhysicalDevice;
        vk::Device LogicalDevice;

    private:
        bool SelectPhysicalDevice();

        VulkanContext* Context; // Cached pointer to backend context

        VulkanSwapchainSupportInfo SwapchainSupport;
    
        int32_t GraphicsQueueIndex;
        int32_t PresentQueueIndex;
        int32_t TransferQueueIndex;
        int32_t ComputeQueueIndex;

        vk::Queue GraphicsQueue;
        vk::Queue PresentQueue;
        vk::Queue TransferQueue;
        vk::Queue ComputeQueue;
        
        vk::Format DepthFormat;

        vk::CommandPool GraphicsCommandPool;

        vk::PhysicalDeviceProperties Properties;
        vk::PhysicalDeviceFeatures Features;
        vk::PhysicalDeviceMemoryProperties MemoryProperties;
};