#include "RendererFrontend.h"

#include "renderer/vulkan/VulkanBackend.h"
#include "core/Logger.h"

bool Renderer::Initialize(const std::string& AppName)
{
    Backend = std::make_unique<VulkanBackend>(); // TODO: make some more flexible way to choose renderer backend
    if (!Backend->Initialize(AppName))
    {
        Logger::Get()->MFatal("Renderer backend failed to initialize. Shutting down...");
        return false;
    }

    return true;
}

void Renderer::Shutdown()
{
    if (Backend)
    {
        Backend->Shutdown();
    }
}

void Renderer::OnResized(uint16_t NewWidth, uint16_t NewHeight)
{
    if (Backend)
    {
        Backend->OnResized(NewWidth, NewHeight);
    }
    else
    {
        Logger::Get()->MWarning("Renderer backend does not exist on calling RendererOnResized: %i %i", NewWidth, NewHeight);
    }
}

bool Renderer::DrawFrame(RenderPacket* Packet)
{
    if (BeginFrame(Packet->DeltaTime))
    {
        bool Result = EndFrame(Packet->DeltaTime);

        if (!Result)
        {
            Logger::Get()->MError("Renderer EndFrame failed. Shutting down...");
            return false;
        }
    }

    return true;
}

bool Renderer::BeginFrame(float DeltaTime)
{
    if (!Backend)
    {
        Logger::Get()->MWarning("Renderer backend does not exist on calling BeginFrame");
        return false;
    }

    return Backend->BeginFrame(DeltaTime);
}

bool Renderer::EndFrame(float DeltaTime)
{
    if (!Backend)
    {
        Logger::Get()->MWarning("Renderer backend does not exist on calling EndFrame");
        return false;
    }
    
    return Backend->EndFrame(DeltaTime);
}
