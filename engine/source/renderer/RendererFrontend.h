#pragma once

#include "RendererTypes.inl"
#include "RendererBackend.h"

class Renderer
{
    public:
        static Renderer* Get();

        static bool Initialize(size_t* outMemReq, void* Ptr,
                               const std::string& AppName, 
                               const uint32_t FramebufferWidth, const uint32_t FramebufferHeight);
        static void Shutdown();

        void OnResized(uint16_t NewWidth, uint16_t NewHeight);
        bool DrawFrame(RenderPacket* Packet);

    private:
        std::unique_ptr<RendererBackend> Backend;

        bool BeginFrame(float DeltaTime);
        bool EndFrame(float DeltaTime);

        static Renderer* Instance;
};