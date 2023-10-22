#include "VulkanBackend.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "platform/Platform.h"
#include "core/Logger.h"
#include "core/MlokUtils.h"
#include "core/Asserts.h"

#include "VulkanDevice.h"

#include <memory>

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
        MlokDebug("Searching for layer: %s...", ReqLayer);
        bool bFound = false;
        for (const auto& LayerToCheck : LayerProperties)
        {
            if (MlokUtils::StringsAreEqual(ReqLayer, LayerToCheck.layerName.data()))
            {
                bFound = true;
                MlokDebug("Found.");
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
    if (!CreateInstance(InstanceCreateInfo))
    {
        return false;
    }

#ifndef NDEBUG
    if (!CreateDebugger())
    {
        return false;
    }
#endif

    if (!CreateSurface())
    {
        return false;
    }

    if (!CreateDevice())
    {
         return false;
    }

    return true;
}

void VulkanBackend::Shutdown()
{
    MlokInfo("Destroying Vulkan Device...");
    Context.pDevice->Destroy();

    MlokInfo("Destroying Vulkan Surface...");
    Context.pInstance->destroySurfaceKHR(Context.Surface, Context.Allocator);

#ifndef NDEBUG
    MlokInfo("Destroying Vulkan Debugger...");
    if (Context.DebugMessenger)
    {
        Context.pInstance->destroyDebugUtilsMessengerEXT(Context.DebugMessenger, Context.Allocator);
    }
#endif

    MlokInfo("Destroying Vulkan Instance...");
    Context.pInstance->destroy(Context.Allocator);
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

bool VulkanBackend::CreateInstance(const vk::InstanceCreateInfo& CreateInfo)
{
    try
    {
        Context.pInstance = std::make_unique<vk::Instance>(vk::createInstance(CreateInfo, Context.Allocator));
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
    MlokInfo("Vulkan Instance created.");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(*Context.pInstance);

    return true;
}

bool VulkanBackend::CreateDebugger()
{
    MlokInfo("Creating Vulkan Debugger...");

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
        Context.DebugMessenger = Context.pInstance->createDebugUtilsMessengerEXT(DebugCreateInfo, Context.Allocator);;
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

    return true;
}

bool VulkanBackend::CreateSurface()
{
    MlokInfo("Creating Vulkan Surface...");
    if (!Platform::Get()->CreateVulkanSurface(&Context))
    {
        MlokFatal("Failed to create Vulkan Surface");
        return false;
    }
    MlokInfo("Vulkan Surface created.");

    return true;
}

bool VulkanBackend::CreateDevice()
{
    MlokInfo("Creating Vulkan Device...");
    Context.pDevice = std::make_unique<VulkanDevice>(&Context);  
    if (Context.pDevice->LogicalDevice)
    {  
        VULKAN_HPP_DEFAULT_DISPATCHER.init(Context.pDevice->LogicalDevice);
    }
    MlokInfo("Vulkan Device created.");

    return true;
}