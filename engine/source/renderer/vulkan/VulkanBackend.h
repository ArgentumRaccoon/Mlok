#pragma once

#include "renderer/RendererBackend.h"
#include "VulkanTypes.inl"
#include "VulkanContext.h"

class VulkanBackend : public RendererBackend
{
    public:
        virtual bool Initialize(const std::string& AppName, const uint32_t FramebufferWidth, const uint32_t FramebufferHeight) override;
        virtual void Shutdown() override;

        virtual void OnResized(uint16_t NewWidth, uint16_t NewHeight) override;
        virtual bool BeginFrame(float DeltaTime) override;
        virtual bool EndFrame(float DeltaTime) override;

    private:
        VulkanContext Context;

        uint32_t CachedFramebufferWidth { 0 };
        uint32_t CachedFramebufferHeight { 0 };

        // Dynamic loader for ext calls
        vk::DynamicLoader dl;

    private:
        bool CreateInstance(const vk::InstanceCreateInfo& CreateInfo);
        bool CreateDebugger();
        bool CreateSurface();
        bool CreateDevice();
        bool CreateSwapchain();
        bool CreateMainRenderPass();
        void CreateCommandBuffers();
        void CreateSyncObjects();
        bool CreateObjectShader();

        bool RecreateSwapchain();
};