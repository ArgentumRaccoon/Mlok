#include "VulkanBackend.h"

#include "platform/Platform.h"
#include "core/Logger.h"

#include "core/MlokUtils.h"

bool VulkanBackend::Initialize(const std::string& AppName, const uint32_t FramebufferWidth, const uint32_t FramebufferHeight)
{
    Context.Allocator = nullptr;

    Context.FramebufferWidth  = FramebufferWidth  != 0 ? FramebufferWidth  : 1280;
    Context.FramebufferHeight = FramebufferHeight != 0 ? FramebufferHeight : 720;
    CachedFramebufferWidth  = 0;
    CachedFramebufferHeight = 0;

    vk::ApplicationInfo ApplicationInfo(AppName.c_str(), VK_MAKE_VERSION(1, 0, 0), "Mlok Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3);

    std::vector<const char*> RequiredExtensions;
    Platform::Get()->GetRequiredExtensionNames(RequiredExtensions);

#ifdef _DEBUG
    RequiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    MlokDebug("Required Vulkan Extensions: ");
    for (const auto& ExtName : RequiredExtensions)
    {
        MlokDebug(ExtName);
    }
#endif

    std::vector<char*> RequiredValidationLayerNames;

#ifdef _DEBUG
    RequiredValidationLayerNames.push_back("VK_LAYER_KHRONOS_validation");
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
    MlokDebug("Vulkan Instance created.");

    return true;
}

void VulkanBackend::Shutdown()
{
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