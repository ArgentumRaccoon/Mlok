#include "VulkanBackend.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "platform/Platform.h"
#include "core/Logger.h"
#include "core/MlokUtils.h"
#include "core/Asserts.h"

#include "VulkanUtils.h"

#include <memory>

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
                                               VkDebugUtilsMessageSeverityFlagsEXT MessageTypes,
                                               const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
                                               void* UserData);

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
    std::vector<vk::LayerProperties> LayerProperties = vk::enumerateInstanceLayerProperties().value;
    
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

    if (!CreateInstance(vk::InstanceCreateInfo()
                        .setPApplicationInfo(&ApplicationInfo)
                        .setPEnabledLayerNames(RequiredValidationLayerNames)
                        .setPEnabledExtensionNames(RequiredExtensions)))
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

    if (!CreateSwapchain())
    {
        return false;
    }

    if (!CreateMainRenderPass())
    {
        return false;
    }

    Context.pSwapchain->RegenerateFramebuffers(Context.pMainRenderPass.get());

    CreateCommandBuffers();

    CreateSyncObjects();

    if (!CreateObjectShader())
    {
        return false;
    }

    return true;
}

void VulkanBackend::Shutdown()
{
    Context.pDevice->LogicalDevice.waitIdle();

    MlokInfo("Destroying Vulkan Object Shader...");
    Context.ObjectShader->Destroy();

    MlokInfo("Destroying Vulkan Sync Objects...");
    auto DestroySemaphore = [&](vk::Semaphore& SemaphoreToDestroy) {
        if (SemaphoreToDestroy) Context.pDevice->LogicalDevice.destroySemaphore(SemaphoreToDestroy, Context.Allocator);
    };    
    std::for_each(Context.ImageAvailableSemaphores.begin(),
                  Context.ImageAvailableSemaphores.end(),
                  DestroySemaphore);
    Context.ImageAvailableSemaphores.clear();    
    std::for_each(Context.QueueCompleteSemaphores.begin(),
                  Context.QueueCompleteSemaphores.end(),
                  DestroySemaphore);
    Context.QueueCompleteSemaphores.clear();
    Context.InFlightFences.clear();
    Context.ImagesInFlight.clear();

    MlokInfo("Freeing Vulkan CommandBuffers...");
    Context.GraphicsCommandBuffers.clear();

    MlokInfo("Destroying Vulkan Framebuffers...");
    Context.pSwapchain->DestroyFramebuffers();

    MlokInfo("Destroying Main Render Pass...");
    Context.pMainRenderPass->Destroy();

    MlokInfo("Destroying Vulkan Swapchain...");
    Context.pSwapchain->Destroy();

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

void VulkanBackend::OnResized(uint16_t NewWidth, uint16_t NewHeight)
{
    CachedFramebufferWidth  = NewWidth;
    CachedFramebufferHeight = NewHeight;
    Context.FramebufferSizeGeneration++;

    MlokInfo("Vulkan Renderer Backend resized event: w/h/g\n: %i/%i/%llu", NewWidth, NewHeight, Context.FramebufferSizeGeneration);
}

bool VulkanBackend::BeginFrame(float DeltaTime)
{
    if (Context.bRecreatingSwapchain)
    {
        vk::Result Result = Context.pDevice->LogicalDevice.waitIdle();
        if (!VulkanUtils::ResultIsSuccess(Result))
        {
            MlokFatal("Vulkan Backend Device Wait Idle (1) failed: %s", VulkanUtils::VulkanResultString(Result, true).c_str());
            return false;    
        }
        
        MlokInfo("Recreating Swapchain...");
        return false;
    }

    if (Context.FramebufferSizeGeneration != Context.FramebufferSizeLastGeneration)
    {
        vk::Result Result = Context.pDevice->LogicalDevice.waitIdle();
        if (!VulkanUtils::ResultIsSuccess(Result))
        {
            MlokFatal("Vulkan Backend Device Wait Idle (2) failed: %s", VulkanUtils::VulkanResultString(Result, true).c_str());
            return false;    
        }

        if (!RecreateSwapchain())
        {
            return false;
        }

        MlokInfo("Resized");
        return false;
    }

    if (!Context.InFlightFences[Context.CurrentFrame].Wait(UINT64_MAX))
    {
        MlokWarning("In flight fence wait failure");
        return false;
    }

    if (!Context.pSwapchain->AcquireNextImageIndex(UINT64_MAX, 
                                                  Context.ImageAvailableSemaphores[Context.CurrentFrame], 
                                                  VK_NULL_HANDLE, 
                                                  &Context.ImageIndex))
    {
        return false;
    }

    auto& CommandBuffer = Context.GraphicsCommandBuffers[Context.ImageIndex];
    CommandBuffer.Reset();
    CommandBuffer.Begin(false, false, false);

    vk::Viewport Viewport {};
    Viewport.setX(0.f)
            .setY(static_cast<float>(Context.FramebufferHeight))
            .setWidth(static_cast<float>(Context.FramebufferWidth))
            .setHeight(-static_cast<float>(Context.FramebufferHeight))
            .setMinDepth(0.f)
            .setMaxDepth(1.f);
    
    vk::Rect2D Scissor {};
    Scissor.setOffset({ 0, 0 })
           .setExtent({ Context.FramebufferWidth, Context.FramebufferHeight });

    CommandBuffer.Get()->setViewport(0, 1, &Viewport);
    CommandBuffer.Get()->setScissor(0, 1, &Scissor);

    Context.pMainRenderPass->SetWidth(Context.FramebufferWidth);
    Context.pMainRenderPass->SetHeight(Context.FramebufferHeight);

    Context.pMainRenderPass->Begin(&CommandBuffer, *Context.pSwapchain->GetFramebuffer(Context.ImageIndex).Get());

    return true;
}

bool VulkanBackend::EndFrame(float DeltaTime)
{
    auto& CommandBuffer = Context.GraphicsCommandBuffers[Context.ImageIndex];

    Context.pMainRenderPass->End(&CommandBuffer);

    CommandBuffer.End();

    if (Context.ImagesInFlight[Context.ImageIndex])
    {
        Context.ImagesInFlight[Context.ImageIndex]->Wait(UINT64_MAX);
    }

    Context.ImagesInFlight[Context.ImageIndex] = &Context.InFlightFences[Context.CurrentFrame];

    Context.InFlightFences[Context.CurrentFrame].Reset();

    std::vector<vk::PipelineStageFlags> PipelineStageFlags = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo SubmitInfo {};
    SubmitInfo.setCommandBufferCount(1)
              .setPCommandBuffers(CommandBuffer.Get())
              .setSignalSemaphoreCount(1)
              .setPSignalSemaphores(&Context.QueueCompleteSemaphores[Context.CurrentFrame])
              .setWaitSemaphoreCount(1)
              .setPWaitSemaphores(&Context.ImageAvailableSemaphores[Context.CurrentFrame])
              .setWaitDstStageMask(PipelineStageFlags);

    vk::Result SubmitResult = Context.pDevice->GetGraphicsQueue().submit(1, &SubmitInfo, *Context.InFlightFences[Context.CurrentFrame].Get());
    if (SubmitResult != vk::Result::eSuccess)
    {
        MlokError("Failed to submit Graphics Queue: %s", VulkanUtils::VulkanResultString(SubmitResult, true).c_str());
        return false;
    }

    CommandBuffer.UpdateSubmitted();

    Context.pSwapchain->Present(Context.pDevice->GetGraphicsQueue(),
                                Context.pDevice->GetPresentQueue(),
                                Context.QueueCompleteSemaphores[Context.CurrentFrame],
                                Context.ImageIndex);

    FrameCount++;

    return true;
}

void VulkanBackend::UpdateGlobalState(Mat4 Projection, Mat4 View, Vec3 ViewPosition, Vec4 AmbientColor, int32_t Mode)
{
    VulkanCommandBuffer* CommandBuffer = &Context.GraphicsCommandBuffers[Context.ImageIndex];
    
    Context.ObjectShader->Use();

    Context.ObjectShader->GetGlobalUBO().Projection = Projection;
    Context.ObjectShader->GetGlobalUBO().View = View;

    // Here will be some more UBO properties

    Context.ObjectShader->UpdateGlobalState();
}

bool VulkanBackend::CreateInstance(const vk::InstanceCreateInfo& CreateInfo)
{
    Context.pInstance = std::make_unique<vk::Instance>(vk::createInstance(CreateInfo, Context.Allocator).value);
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
                               
    vk::DebugUtilsMessengerCreateInfoEXT DebugCreateInfo {};
    DebugCreateInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagsEXT(MessageSeverity))
                   .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
                   .setPfnUserCallback(VkDebugCallback);
    
    const auto& CreateResult = Context.pInstance->createDebugUtilsMessengerEXT(DebugCreateInfo, Context.Allocator);
    
    if (!VulkanUtils::ResultIsSuccess(CreateResult.result))
    {
        MlokFatal("Failed to create Vulkan Debugger: %s", VulkanUtils::VulkanResultString(CreateResult.result, true).c_str());
        return false;
    }

    Context.DebugMessenger = CreateResult.value;
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

bool VulkanBackend::CreateSwapchain()
{
    MlokInfo("Creating Vulkan Swapchain...");
    Context.pSwapchain = std::make_unique<VulkanSwapchain>(&Context, Context.FramebufferWidth, Context.FramebufferHeight);
    MlokInfo("Vulkan Swapchain created.");

    return true;
}

bool VulkanBackend::CreateMainRenderPass()
{
    MlokInfo("Creating Main Render Pass...");
    Context.pMainRenderPass = std::make_unique<VulkanRenderPass>(&Context,
                                                                 0.f, 0.f, Context.FramebufferWidth, Context.FramebufferHeight,
                                                                 0.1f, 0.1f, 0.25f, 1.f,
                                                                 1.f, 0);
    MlokInfo("Main Render Pass created.");

    return true;
}

void VulkanBackend::CreateCommandBuffers()
{
    if (Context.GraphicsCommandBuffers.empty())
    {
        Context.GraphicsCommandBuffers.resize(Context.pSwapchain->GetImageCount());
    }

    for (auto& CommandBuffer : Context.GraphicsCommandBuffers)
    {
        CommandBuffer.Free();
        Platform::PlatformZeroMemory(&CommandBuffer, sizeof(VulkanCommandBuffer));
        CommandBuffer.Allocate(&Context, Context.pDevice->GetGraphicsCommandPool(), true);
    }
}

void VulkanBackend::CreateSyncObjects()
{
    Context.ImageAvailableSemaphores.clear();
    Context.QueueCompleteSemaphores.clear();
    Context.InFlightFences.clear();
    Context.ImagesInFlight.clear();

    Context.ImageAvailableSemaphores.resize(Context.pSwapchain->GetMaxFramesInFlight());
    Context.QueueCompleteSemaphores.resize(Context.pSwapchain->GetMaxFramesInFlight());
    Context.InFlightFences.resize(Context.pSwapchain->GetMaxFramesInFlight());

    for (size_t i = 0; i < Context.pSwapchain->GetMaxFramesInFlight(); ++i)
    {
        vk::SemaphoreCreateInfo SemaphoreCreateInfo {};
        Context.ImageAvailableSemaphores[i] = Context.pDevice->LogicalDevice.createSemaphore(SemaphoreCreateInfo, Context.Allocator).value;
        Context.QueueCompleteSemaphores[i] = Context.pDevice->LogicalDevice.createSemaphore(SemaphoreCreateInfo, Context.Allocator).value;

        Context.InFlightFences[i].Create(&Context, true);// .push_back(VulkanFence(&Context, true));
    }

    Context.ImagesInFlight.resize(Context.pSwapchain->GetImageCount(), nullptr);
}

bool VulkanBackend::CreateObjectShader()
{
    MlokInfo("Creating Vulkan Object Shader...");
    Context.ObjectShader = std::make_unique<VulkanObjectShader>(&Context);
    MlokInfo("Vulkan Object Shader created.");

    return true;
}

bool VulkanBackend::RecreateSwapchain()
{
    if (Context.bRecreatingSwapchain)
    {
        MlokDebug("RecreateSwapchain called during continuous swapchain recreation.");
        return false;
    }

    if (Context.FramebufferWidth == 0 || Context.FramebufferHeight == 0)
    {
        MlokDebug("RecreateSwapchain called for window dimension < 1.");
        return false;
    }

    Context.bRecreatingSwapchain = true;

    Context.pDevice->LogicalDevice.waitIdle();

    std::for_each(Context.ImagesInFlight.begin(),
                  Context.ImagesInFlight.end(),
                  [&](VulkanFence* ImageFence) { ImageFence = nullptr; });

    Context.pSwapchain->Recreate(Context.FramebufferWidth, Context.FramebufferHeight);

    Context.FramebufferWidth  = CachedFramebufferWidth;
    Context.FramebufferHeight = CachedFramebufferHeight;
    Context.pMainRenderPass->SetWidth(static_cast<float>(Context.FramebufferWidth));
    Context.pMainRenderPass->SetHeight(static_cast<float>(Context.FramebufferHeight));
    CachedFramebufferWidth  = 0;
    CachedFramebufferHeight = 0;

    Context.FramebufferSizeLastGeneration = Context.FramebufferSizeGeneration;

    std::for_each(Context.GraphicsCommandBuffers.begin(),
                  Context.GraphicsCommandBuffers.end(),
                  [&](VulkanCommandBuffer& CB) { CB.Free(); });
    Context.pSwapchain->DestroyFramebuffers();

    Context.pMainRenderPass->SetX(0);
    Context.pMainRenderPass->SetY(0);
    Context.pMainRenderPass->SetWidth(Context.FramebufferWidth);
    Context.pMainRenderPass->SetHeight(Context.FramebufferHeight);

    Context.pSwapchain->RegenerateFramebuffers(Context.pMainRenderPass.get());

    CreateCommandBuffers();

    Context.bRecreatingSwapchain = false;

    return true;
}

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