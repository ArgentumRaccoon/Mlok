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
        VulkanDevice(const VulkanDevice&) = delete;        
        VulkanDevice& operator=(const VulkanDevice&) = delete;
        ~VulkanDevice();

        bool Create(VulkanContext* Context);
        void Destroy();

        void QuerySwapchainSupport(const vk::PhysicalDevice& inPhysicalDevice,
                                   VkSurfaceKHR Surface);
        bool DetectDepthFormat();

        const vk::Format& GetDepthFormat() const { return DepthFormat; }
        void SetDepthFormat(const vk::Format NewFormat) { DepthFormat = NewFormat; }

        const VulkanSwapchainSupportInfo& GetSwapchainSupport() const { return SwapchainSupport; }

        const int32_t GetGraphicsQueueIndex() const { return GraphicsQueueIndex; }
        const int32_t GetPresentQueueIndex()  const { return PresentQueueIndex;  }
        const int32_t GetTransferQueueIndex() const { return TransferQueueIndex; }
        const int32_t GetComputeQueueIndex()  const { return ComputeQueueIndex;  }

        vk::CommandPool& GetGraphicsCommandPool() { return GraphicsCommandPool; }

        vk::PhysicalDevice PhysicalDevice;
        vk::Device LogicalDevice;

    private:
        bool PhysicalDeviceMeetsRequirements(const vk::PhysicalDevice& PhysicalDeviceToCheck,
                                             vk::SurfaceKHR Surface,
                                             const vk::PhysicalDeviceProperties* Properties,
                                             const vk::PhysicalDeviceFeatures* Features,
                                             const VulkanPhysicalDeviceRequirements* Requirements,
                                             VulkanPhysicalDeviceQueueFamilyInfo* OutQueueFamilyInfo);
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