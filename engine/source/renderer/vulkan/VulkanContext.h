#pragma once

#include "VulkanTypes.inl"

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderPass.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFence.h"
#include "shaders/VulkanObjectShader.h"

#include <memory>

class VulkanContext
{
    public:
        std::unique_ptr<vk::Instance> pInstance;
        vk::AllocationCallbacks* Allocator;
        vk::SurfaceKHR Surface;
#ifndef NDEBUG
        vk::DebugUtilsMessengerEXT DebugMessenger;
#endif
        
        std::unique_ptr<VulkanDevice> pDevice;
        std::unique_ptr<VulkanSwapchain> pSwapchain;
        std::unique_ptr<VulkanRenderPass> pMainRenderPass;

        std::vector<VulkanCommandBuffer> GraphicsCommandBuffers;

        std::vector<vk::Semaphore> ImageAvailableSemaphores;
        std::vector<vk::Semaphore> QueueCompleteSemaphores;

        std::vector<VulkanFence> InFlightFences;
        std::vector<VulkanFence*> ImagesInFlight;

        std::unique_ptr<VulkanObjectShader> ObjectShader;

        uint32_t FramebufferWidth;
        uint32_t FramebufferHeight;

        uint64_t FramebufferSizeGeneration;
        uint64_t FramebufferSizeLastGeneration;

        uint32_t CurrentFrame;
        uint32_t ImageIndex;

        bool bRecreatingSwapchain;

        virtual int32_t FindMemoryIndex(uint32_t TypeFilter, vk::MemoryPropertyFlags PropertyFlags);
};