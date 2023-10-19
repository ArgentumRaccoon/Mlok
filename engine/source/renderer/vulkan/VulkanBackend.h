#pragma once

#include "renderer/RendererBackend.h"
#include "VulkanTypes.inl"

class VulkanBackend : public RendererBackend
{
    public:
        virtual bool Initialize(const std::string& AppName, const uint32_t FramebufferWidth, const uint32_t FramebufferHeight) override;
        virtual void Shutdown() override;

        virtual void OnResized(uint16_t NewWidth, uint16_t Height) override;
        virtual bool BeginFrame(float DeltaTime) override;
        virtual bool EndFrame(float DeltaTime) override;

    private:
        VulkanContext Context;

        uint32_t CachedFramebufferWidth { 0 };
        uint32_t CachedFramebufferHeight { 0 };
};