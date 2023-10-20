#include "RendererFrontend.h"

#include "renderer/vulkan/VulkanBackend.h"
#include "core/Logger.h"

Renderer* Renderer::Instance = nullptr;

Renderer* Renderer::Get()
{
    return Instance;
}

bool Renderer::Initialize(size_t* outMemReq, void* Ptr,
                          const std::string& AppName, 
                          const uint32_t FramebufferWidth, const uint32_t FramebufferHeight)
{    
    *outMemReq = sizeof(Renderer);
    if (Ptr == nullptr)
    {
        return true;
    }

    Instance = static_cast<Renderer*>(Ptr);

    Instance->Backend = std::make_unique<VulkanBackend>(); // TODO: make some more flexible way to choose renderer backend
    if (!Instance->Backend->Initialize(AppName, FramebufferWidth, FramebufferHeight))
    {
        MlokFatal("Renderer backend failed to initialize. Shutting down...");
        return false;
    }

    return true;
}

void Renderer::Shutdown()
{
    if (Instance->Backend)
    {
        Instance->Backend->Shutdown();
    }

    Instance = nullptr;
}

void Renderer::OnResized(uint16_t NewWidth, uint16_t NewHeight)
{
    if (Backend)
    {
        Backend->OnResized(NewWidth, NewHeight);
    }
    else
    {
        MlokWarning("Renderer backend does not exist on calling RendererOnResized: %i %i", NewWidth, NewHeight);
    }
}

bool Renderer::DrawFrame(RenderPacket* Packet)
{
    if (BeginFrame(Packet->DeltaTime))
    {
        bool bResult = EndFrame(Packet->DeltaTime);

        if (!bResult)
        {
            MlokError("Renderer EndFrame failed. Shutting down...");
            return false;
        }
    }

    return true;
}

bool Renderer::BeginFrame(float DeltaTime)
{
    if (!Backend)
    {
        MlokWarning("Renderer backend does not exist on calling BeginFrame");
        return false;
    }

    return Backend->BeginFrame(DeltaTime);
}

bool Renderer::EndFrame(float DeltaTime)
{
    if (!Backend)
    {
        MlokWarning("Renderer backend does not exist on calling EndFrame");
        return false;
    }
    
    return Backend->EndFrame(DeltaTime);
}
