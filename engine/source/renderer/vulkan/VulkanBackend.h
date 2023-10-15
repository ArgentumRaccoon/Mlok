#pragma once

#include "renderer/RendererBackend.h"

class VulkanBackend : public RendererBackend
{
    public:
        virtual bool Initialize(const std::string& AppName);
        virtual void Shutdown();

        virtual void OnResized(uint16_t NewWidth, uint16_t Height);
        virtual bool BeginFrame(float DeltaTime);
        virtual bool EndFrame(float DeltaTime);
};