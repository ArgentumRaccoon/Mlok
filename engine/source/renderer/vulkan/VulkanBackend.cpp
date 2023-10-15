#include "VulkanBackend.h"

bool VulkanBackend::Initialize(const std::string& AppName)
{
    return true;
}

void VulkanBackend::Shutdown()
{

}

void VulkanBackend::OnResized(uint16_t NewWidth, uint16_t Height)
{

}

bool VulkanBackend::BeginFrame(float DeltaTime)
{
    return true;
}

bool VulkanBackend::EndFrame(float DeltaTime)
{
    FrameCount++;
    return true;
}