#include "VulkanDevice.h"

#include "core/Logger.h"
#include "core/MlokUtils.h"
#include "platform/Platform.h"

#include "VulkanContext.h"

#include <algorithm>

VulkanDevice::VulkanDevice(VulkanContext* inContext)
    : Context { inContext }
{
    Create(Context);
}

VulkanDevice::~VulkanDevice()
{
    Destroy();
    Context = nullptr;
}

bool VulkanDevice::Create(VulkanContext* inContext)
{
    if (Context != inContext)
    {
        Context = inContext;
    }

    if (!SelectPhysicalDevice())
    {
        MlokFatal("Cannot select Vulkan Physical Device");
        return false;
    }

    MlokInfo("Creating Logical Device...");
    bool bPresentSharesGraphicsQueue  = GraphicsQueueIndex == PresentQueueIndex;
    bool bTransferSharesGraphicsQueue = GraphicsQueueIndex == TransferQueueIndex;
    // TODO: No compute queue presented currently
    // bool bComputeSharesGraphicsQueue = GraphicsQueueIndex == ComputeQueueIndex;

    std::vector<uint32_t> Indices;
    Indices.push_back(GraphicsQueueIndex);
    if (!bPresentSharesGraphicsQueue)
    {
        Indices.push_back(PresentQueueIndex);
    }
    if (!bTransferSharesGraphicsQueue)
    {
        Indices.push_back(TransferQueueIndex);
    }
    // if (!bComputeSharesGraphicsQueue)
    // {
    //     Indices.push_back(ComputeQueueIndex);
    // }

    std::vector<vk::DeviceQueueCreateInfo> QueueCreateInfos;
    std::vector<float> QueuePriorities { 1.f };

    for (auto& QueueFamilyIndex : Indices)
    {
        QueueCreateInfos.push_back(vk::DeviceQueueCreateInfo()
                                   .setQueueFamilyIndex(QueueFamilyIndex)
                                   .setQueuePriorities(QueuePriorities));
    }

    vk::PhysicalDeviceFeatures DeviceFeatures {};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;

    std::vector<const char*> EnabledExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    vk::DeviceCreateInfo CreateInfo {};
    CreateInfo.setQueueCreateInfos(QueueCreateInfos)
              .setPEnabledExtensionNames(EnabledExtensions)
              .setPEnabledFeatures(&Features);

    try
    {
        LogicalDevice = PhysicalDevice.createDevice(CreateInfo, Context->Allocator);
    }
    catch(const vk::SystemError& err)
    {
        MlokFatal("Failed to create Vulkan Device: %s", err.what());
        return false;    
    }
    catch(...)
    {
        MlokFatal("Failed to create Vulkan Device: Unknown Error");
        return false;
    }
    MlokInfo("Logical Device created");

    GraphicsQueue = LogicalDevice.getQueue(GraphicsQueueIndex, 0);
    PresentQueue  = LogicalDevice.getQueue(PresentQueueIndex,  0);
    TransferQueue = LogicalDevice.getQueue(TransferQueueIndex, 0);
    // ComputeQueue  = LogicalDevice.getQueue(ComputeQueueIndex,  0);

    vk::CommandPoolCreateInfo PoolCreateInfo {};
    PoolCreateInfo.setQueueFamilyIndex(GraphicsQueueIndex)
                  .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    GraphicsCommandPool = LogicalDevice.createCommandPool(PoolCreateInfo, Context->Allocator);
    MlokInfo("Graphics Command Pool created.");

    return true;
}

void VulkanDevice::Destroy()
{
    MlokInfo("Destroying Command Pools...");
    if (GraphicsCommandPool)
    {
        LogicalDevice.destroyCommandPool(GraphicsCommandPool, Context->Allocator);
    }

    MlokInfo("Destroying Logical Device...");
    if (LogicalDevice)
    {
        LogicalDevice.destroy(Context->Allocator);
        LogicalDevice = nullptr;
    }

    MlokInfo("Releasing Physical Device resources...");
    PhysicalDevice = nullptr;

    if (!SwapchainSupport.Formats.empty())
    {
        SwapchainSupport.Formats.clear();
    }

    if (!SwapchainSupport.PresentModes.empty())
    {
        SwapchainSupport.PresentModes.clear();
    }

    SwapchainSupport.SurfaceCapabilities = vk::SurfaceCapabilitiesKHR {};

    GraphicsQueueIndex = -1;
    PresentQueueIndex  = -1;
    TransferQueueIndex = -1;
    ComputeQueueIndex  = -1;

    Context = nullptr;
}

void VulkanDevice::QuerySwapchainSupport(const vk::PhysicalDevice& inPhysicalDevice,
                                         VkSurfaceKHR Surface)
{
    SwapchainSupport.SurfaceCapabilities = inPhysicalDevice.getSurfaceCapabilitiesKHR(Surface);
    SwapchainSupport.Formats = inPhysicalDevice.getSurfaceFormatsKHR(Surface);
    SwapchainSupport.PresentModes = inPhysicalDevice.getSurfacePresentModesKHR(Surface);
}

bool VulkanDevice::DetectDepthFormat()
{
    std::vector<vk::Format> Candidates = { vk::Format::eD32Sfloat,
                                           vk::Format::eD32SfloatS8Uint,
                                           vk::Format::eD24UnormS8Uint };

    vk::FormatFeatureFlagBits Flags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

    for (auto Format : Candidates)
    {
        vk::FormatProperties Properties = PhysicalDevice.getFormatProperties(Format);
        if ((Properties.linearTilingFeatures  & Flags) == Flags ||
            (Properties.optimalTilingFeatures & Flags) == Flags)
        {
            DepthFormat = Format;
            return true;
        }
    }

    return false;
}

bool VulkanDevice::PhysicalDeviceMeetsRequirements(const vk::PhysicalDevice& PhysicalDeviceToCheck,
                                                   vk::SurfaceKHR Surface,
                                                   const vk::PhysicalDeviceProperties* Properties,
                                                   const vk::PhysicalDeviceFeatures* Features,
                                                   const VulkanPhysicalDeviceRequirements* Requirements,
                                                   VulkanPhysicalDeviceQueueFamilyInfo* OutQueueFamilyInfo)
{
    OutQueueFamilyInfo->GraphicsFamilyIndex = -1;
    OutQueueFamilyInfo->PresentFamilyIndex  = -1;
    OutQueueFamilyInfo->TransferFamilyIndex = -1;
    OutQueueFamilyInfo->ComputeFamilyIndex  = -1;

    if (Requirements->bDiscreteGPU)
    {
        if (Properties->deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
        {
            MlokInfo("Device is not a discrete GPU.");
            return false;
        }
    }

    std::vector<vk::QueueFamilyProperties> QueueFamilies = PhysicalDeviceToCheck.getQueueFamilyProperties();

    MlokInfo("GRAPHICS | PRESENT | TRANSFER | COMPUTE | NAME");
    uint8_t MinTransferScore = 255;
    for (size_t i = 0; i < QueueFamilies.size(); ++i)
    {
        // Searching for Transfer family with min number of queue flags (i.e. the ideal gueue family has only transfer bit)
        const vk::QueueFamilyProperties& QueueFamily = QueueFamilies[i];
        uint8_t CurrentTransferScore = 0;

        if (QueueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            OutQueueFamilyInfo->GraphicsFamilyIndex = (uint32_t)i;
            ++CurrentTransferScore;
        }

        if (QueueFamily.queueFlags & vk::QueueFlagBits::eCompute)
        {
            OutQueueFamilyInfo->ComputeFamilyIndex = (uint32_t)i;
            ++CurrentTransferScore;
        }

        if (QueueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            if (CurrentTransferScore <= MinTransferScore)
            {
                MinTransferScore = CurrentTransferScore;
                OutQueueFamilyInfo->TransferFamilyIndex = (uint32_t)i;
            }
        }

        if (PhysicalDeviceToCheck.getSurfaceSupportKHR((uint32_t)i, Surface))
        {
            OutQueueFamilyInfo->PresentFamilyIndex = (uint32_t)i;
        }
    }

    MlokInfo("       %d |       %d |        %d |       %d | %s",
             OutQueueFamilyInfo->GraphicsFamilyIndex != -1,
             OutQueueFamilyInfo->PresentFamilyIndex  != -1,
             OutQueueFamilyInfo->TransferFamilyIndex != -1,
             OutQueueFamilyInfo->ComputeFamilyIndex  != -1,
             Properties->deviceName);

    if ((!Requirements->bGraphics || (Requirements->bGraphics && OutQueueFamilyInfo->GraphicsFamilyIndex != -1)) &&
        (!Requirements->bPresent  || (Requirements->bPresent  && OutQueueFamilyInfo->PresentFamilyIndex  != -1)) &&
        (!Requirements->bTransfer || (Requirements->bTransfer && OutQueueFamilyInfo->TransferFamilyIndex != -1)) &&
        (!Requirements->bCompute  || (Requirements->bCompute  && OutQueueFamilyInfo->ComputeFamilyIndex  != -1)))
    {
        MlokInfo("Device %s meets queue requirements.", Properties->deviceName);   
        MlokVerbose("Graphics Family Index: %i", OutQueueFamilyInfo->GraphicsFamilyIndex);
        MlokVerbose("Present Family Index:  %i", OutQueueFamilyInfo->PresentFamilyIndex);
        MlokVerbose("Transfer Family Index: %i", OutQueueFamilyInfo->TransferFamilyIndex);
        MlokVerbose("Compute Family Index:  %i", OutQueueFamilyInfo->ComputeFamilyIndex);

        QuerySwapchainSupport(PhysicalDeviceToCheck, Surface);

        if (SwapchainSupport.Formats.empty() || SwapchainSupport.PresentModes.empty())
        {
            MlokInfo("Required swapchain is not supported for device %s.", Properties->deviceName);
            return false;
        }

        if (!Requirements->DeviceExtensionNames.empty())
        {
            std::vector<vk::ExtensionProperties> AvailableExtensions = PhysicalDeviceToCheck.enumerateDeviceExtensionProperties();
            if (!AvailableExtensions.empty())
            {
                for (auto& Required : Requirements->DeviceExtensionNames)
                {
                    if (std::find_if(AvailableExtensions.begin(), AvailableExtensions.end(),
                                     [&Required](const vk::ExtensionProperties& Available) {
                                        return MlokUtils::StringsAreEqual(Required.c_str(), Available.extensionName);
                                     }) == AvailableExtensions.end())
                    {
                        MlokInfo("Required extension not found: '%s' for device %s", Required.c_str(), Properties->deviceName);
                        return false;
                    }
                }
            }
        }

        if (Requirements->bSamplerAnisotropy && !Features->samplerAnisotropy)
        {
            MlokInfo("Device %s does not support sampler anisotropy", Properties->deviceName);
            return false;
        }

        return true;
    }

    return false;
}

bool VulkanDevice::SelectPhysicalDevice()
{
    for (auto& CandidatePhysicalDevice : Context->pInstance->enumeratePhysicalDevices())
    {
        vk::PhysicalDeviceProperties CandidateProperties = CandidatePhysicalDevice.getProperties();
        vk::PhysicalDeviceFeatures CandidateFeatures = CandidatePhysicalDevice.getFeatures();
        vk::PhysicalDeviceMemoryProperties CandidateMemoryProperties = CandidatePhysicalDevice.getMemoryProperties();

        VulkanPhysicalDeviceRequirements CustomRequirements {};
        CustomRequirements.bGraphics = true;
        CustomRequirements.bPresent = true;
        CustomRequirements.bTransfer = true;
        CustomRequirements.bCompute = false;
        CustomRequirements.bSamplerAnisotropy = true;
        CustomRequirements.bDiscreteGPU = true;

        CustomRequirements.DeviceExtensionNames = {};
        CustomRequirements.DeviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VulkanPhysicalDeviceQueueFamilyInfo QueueInfo {};
        bool bDeviceMeetsRequirements = PhysicalDeviceMeetsRequirements(CandidatePhysicalDevice,
                                                                        Context->Surface,
                                                                        &CandidateProperties,
                                                                        &CandidateFeatures,
                                                                        &CustomRequirements,
                                                                        &QueueInfo);

        if (bDeviceMeetsRequirements)
        {
            MlokInfo("Selected Device: %s.", Properties.deviceName);
            switch (Properties.deviceType)
            {
                default:
                case vk::PhysicalDeviceType::eOther:
                    MlokInfo("GPU type is Unknown");
                    break;
                case vk::PhysicalDeviceType::eIntegratedGpu:
                    MlokInfo("GPU type is Integrated");
                    break;
                case vk::PhysicalDeviceType::eDiscreteGpu:
                    MlokInfo("GPU type is Discrete");
                    break;
                case vk::PhysicalDeviceType::eVirtualGpu:
                    MlokInfo("GPU type is Virtual");
                    break;
                case vk::PhysicalDeviceType::eCpu:
                    MlokInfo("GPU type is CPU");
                    break;
            }

            MlokInfo("GPU Driver version: %d.%d.%d",
                     VK_VERSION_MAJOR(Properties.driverVersion),
                     VK_VERSION_MINOR(Properties.driverVersion),
                     VK_VERSION_PATCH(Properties.driverVersion));

            MlokInfo("Vulkan API version: %d.%d.%d",
                     VK_VERSION_MAJOR(Properties.apiVersion),
                     VK_VERSION_MINOR(Properties.apiVersion),
                     VK_VERSION_PATCH(Properties.apiVersion));
            
            for (auto& MemoryHeap : CandidateMemoryProperties.memoryHeaps)
            {
                float MemorySizeGib = MlokUtils::BytesToGib(MemoryHeap.size);
                if (MemoryHeap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                {
                    MlokInfo("Local GPU memory: %.2f GiB", MemorySizeGib);
                }
                else
                {
                    MlokInfo("Shared System memory: %.2f GiB", MemorySizeGib);
                }
            }

            PhysicalDevice = CandidatePhysicalDevice;
            GraphicsQueueIndex = QueueInfo.GraphicsFamilyIndex;
            PresentQueueIndex = QueueInfo.PresentFamilyIndex;
            TransferQueueIndex = QueueInfo.TransferFamilyIndex;
            ComputeQueueIndex = QueueInfo.ComputeFamilyIndex;

            Properties = CandidateProperties;
            Features = CandidateFeatures;
            MemoryProperties = CandidateMemoryProperties;
            break;
        }
    }

    if (!PhysicalDevice)
    {
        MlokError("No Physical Devices were found which meet the requirements!");
        return false;
    }

    return true;
}