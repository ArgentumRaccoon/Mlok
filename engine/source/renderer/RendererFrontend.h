#pragma once

#include "RendererTypes.inl"
#include "RendererBackend.h"

class Renderer
{
    public:
        static Renderer* Get()
        {
            static Renderer RendererHandle;
            return &RendererHandle;
        }

        bool Initialize(const std::string& AppName);
        void Shutdown();

        void OnResized(uint16_t NewWidth, uint16_t NewHeight);
        bool DrawFrame(RenderPacket* Packet);

    private:
        std::unique_ptr<RendererBackend> Backend;

        bool BeginFrame(float DeltaTime);
        bool EndFrame(float DeltaTime);
};