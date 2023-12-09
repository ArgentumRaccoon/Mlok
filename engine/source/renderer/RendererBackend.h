#pragma once

#include "RendererTypes.inl"

class RendererBackend
{
    public:
        virtual ~RendererBackend() {};

        virtual bool Initialize(const std::string& AppName, const uint32_t FramebufferWidth, const uint32_t FramebufferHeight) = 0;
        virtual void Shutdown() = 0;

        virtual void OnResized(uint16_t NewWidth, uint16_t Height) = 0;
        virtual bool BeginFrame(float DeltaTime) = 0;
        virtual bool EndFrame(float DeltaTime) = 0;

        virtual void UpdateGlobalState(Mat4 Projection, Mat4 View, Vec3 ViewPosition, Vec4 AmbientColor, int32_t Mode) = 0;

    protected:
        uint64_t FrameCount;
};