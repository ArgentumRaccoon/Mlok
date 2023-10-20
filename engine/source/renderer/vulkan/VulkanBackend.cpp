#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "VulkanBackend.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "platform/Platform.h"
#include "core/Logger.h"

#include "core/MlokUtils.h"

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
                                               VkDebugUtilsMessageSeverityFlagsEXT MessageTypes,
                                               const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
                                               void* UserData)
{
    switch (MessageSeverity)
    {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            MlokError(CallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            MlokWarning(CallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            MlokInfo(CallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            MlokVerbose(CallbackData->pMessage);
            break;
    }
    
    return VK_FALSE;
}

bool VulkanBackend::Initialize(const std::string& AppName, const uint32_t FramebufferWidth, const uint32_t FramebufferHeight)
{
    Context.Allocator = nullptr;

    Context.FramebufferWidth  = FramebufferWidth  != 0 ? FramebufferWidth  : 1280;
    Context.FramebufferHeight = FramebufferHeight != 0 ? FramebufferHeight : 720;
    CachedFramebufferWidth  = 0;
    CachedFramebufferHeight = 0;

    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    vk::ApplicationInfo ApplicationInfo(AppName.c_str(), VK_MAKE_VERSION(1, 0, 0), "Mlok Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3);

    std::vector<const char*> RequiredExtensions;
    RequiredExtensions.push_back("VK_KHR_surface");
    Platform::Get()->GetRequiredExtensionNames(RequiredExtensions);

#ifndef NDEBUG
    RequiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    MlokDebug("Required Vulkan Extensions: ");
    for (const auto& ExtName : RequiredExtensions)
    {
        MlokDebug(ExtName);
    }
#endif

    std::vector<const char*> RequiredValidationLayerNames;

#ifndef NDEBUG
    const char* ValidationLayerName = "VK_LAYER_KHRONOS_validation";
    RequiredValidationLayerNames.push_back(ValidationLayerName);
    std::vector<vk::LayerProperties> LayerProperties = vk::enumerateInstanceLayerProperties();
    
    // Check if Required Layers are presented
    for (const auto& ReqLayer : RequiredValidationLayerNames)
    {
        MlokInfo("Searching for layer: %s...", ReqLayer);
        bool bFound = false;
        for (const auto& LayerToCheck : LayerProperties)
        {
            if (MlokUtils::StringsAreEqual(ReqLayer, LayerToCheck.layerName.data()))
            {
                bFound = true;
                MlokInfo("Found.");
                break;
            }
        }

        if (!bFound)
        {
            MlokFatal("Required validation layer is missing: %s", ReqLayer);
            return false;
        }
    }
#endif

    vk::InstanceCreateInfo InstanceCreateInfo({}, &ApplicationInfo, RequiredValidationLayerNames, RequiredExtensions);
    try
    {
        Context.Instance = vk::createInstance(InstanceCreateInfo, Context.Allocator);
    }
    catch(const vk::SystemError& err)
    {
        MlokFatal("Failed to create Vulkan Instance: %s", err.what());
        return false;    
    }
    catch(...)
    {
        MlokFatal("Failed to create Vulkan Instance: Unknown Error");
        return false;
    }
    MlokDebug("Vulkan Instance created.");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(Context.Instance);

#ifndef NDEBUG
    MlokDebug("Creating Vulkan Debugger...");

    uint32_t MessageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; 
                               // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                               // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    vk::DebugUtilsMessengerCreateInfoEXT DebugCreateInfo(vk::DebugUtilsMessengerCreateFlagsEXT(),
                                                         vk::DebugUtilsMessageSeverityFlagsEXT(MessageSeverity),
                                                         vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
                                                         VkDebugCallback);
    
    try
    {
        Context.DebugMessanger = Context.Instance.createDebugUtilsMessengerEXT(DebugCreateInfo, Context.Allocator);
    }
    catch(const vk::SystemError& err)
    {
        MlokFatal("Failed to create Vulkan Debug Messenger: %s", err.what());
        return false;    
    }
    catch(...)
    {
        MlokFatal("Failed to create Vulkan Debug Messenger: Unknown Error");
        return false;
    }

     MlokInfo("Vulkan Debugger created.");
#endif

    return true;
}

void VulkanBackend::Shutdown()
{
#ifndef NDEBUG
    MlokInfo("Destroying Vulkan Debugger...");
    if (Context.DebugMessanger)
    {
        Context.Instance.destroyDebugUtilsMessengerEXT(Context.DebugMessanger, Context.Allocator);
    }
#endif

    MlokDebug("Destroying Vulkan Instance...");
    Context.Instance.destroy(Context.Allocator);
}

void VulkanBackend::OnResized(uint16_t NewWidth, uint16_t Height)
{

}

bool VulkanBackend::BeginFrame(float DeltaTime)
{
    return true;
}

bool VulkanBackend::EndFrame(float DeltaTime)
{
    FrameCount++;
    return true;
}