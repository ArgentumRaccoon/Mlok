#include "VulkanCommandBuffer.h"

#include "VulkanContext.h"

#include "core/Asserts.h"

VulkanCommandBuffer::VulkanCommandBuffer(VulkanContext* inContext,
                                         vk::CommandPool CommandPool,
                                         bool bIsPrimary,
                                         bool bIsSingleUse)
    : Context { inContext }
    , OwningCommandPool { CommandPool }
{
    if (bIsSingleUse)
    {
        AllocateAndBeginSingleUse(inContext, CommandPool);
    }
    else
    {
        Allocate(inContext, CommandPool, bIsPrimary);
    }
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    Free();
}

void VulkanCommandBuffer::Allocate(VulkanContext* inContext,
                                   vk::CommandPool CommandPool,
                                   bool bIsPrimary)
{
    if (Context != inContext)
    {
        Context = inContext;
    }

    if (OwningCommandPool != CommandPool)
    {
        OwningCommandPool = CommandPool;
    }

    vk::CommandBufferAllocateInfo AllocateInfo {};
    AllocateInfo.setCommandPool(CommandPool)
                .setLevel(bIsPrimary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary)
                .setCommandBufferCount(1);

    State = VulkanCommandBufferState::eNotAllocated;
    std::vector<vk::CommandBuffer> AllocatedBuffers = Context->pDevice->LogicalDevice.allocateCommandBuffers(AllocateInfo).value;
    if (!AllocatedBuffers.empty())
    {
        Handle = AllocatedBuffers[0]; // For now always allocating only one command buffer
    }
    State = VulkanCommandBufferState::eReady;
}

void VulkanCommandBuffer::Free()
{
    if (Handle)
    {
        M_ASSERT(OwningCommandPool);

        Context->pDevice->LogicalDevice.freeCommandBuffers(OwningCommandPool, { Handle });
        Handle = nullptr;
        State = VulkanCommandBufferState::eNotAllocated;
    }

    Context = nullptr;
}

void VulkanCommandBuffer::Begin(bool bIsSingleUse,
                                bool bIsRenderPassContinue,
                                bool bIsSimulteneousUse)
{
    vk::CommandBufferBeginInfo BeginInfo;
    vk::CommandBufferUsageFlags Usage;
    if (bIsSingleUse)
    {
        Usage |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    }
    if (bIsRenderPassContinue)
    {
        Usage |= vk::CommandBufferUsageFlagBits::eRenderPassContinue;
    }
    if (bIsSimulteneousUse)
    {
        Usage |= vk::CommandBufferUsageFlagBits::eSimultaneousUse;
    }
    BeginInfo.setFlags(Usage);

    Handle.begin(BeginInfo);
    State = VulkanCommandBufferState::eRecording;
}

void VulkanCommandBuffer::End()
{
    Handle.end();
    State = VulkanCommandBufferState::eRecordingEnded;
}

void VulkanCommandBuffer::UpdateSubmitted()
{
    State = VulkanCommandBufferState::eSubmitted;
}

void VulkanCommandBuffer::UpdateRecording()
{
    State = VulkanCommandBufferState::eRecording;
}

void VulkanCommandBuffer::UpdateInRenderPass()
{
    State = VulkanCommandBufferState::eInRenderPass;
}

void VulkanCommandBuffer::Reset()
{
    State = VulkanCommandBufferState::eReady;
}

void VulkanCommandBuffer::AllocateAndBeginSingleUse(VulkanContext* inContext, vk::CommandPool CommandPool)
{
    const bool bIsPrimary = true; // Always primary when is single use
    Allocate(Context, CommandPool, bIsPrimary);
    Begin(true, false, false);
}

void VulkanCommandBuffer::EndSingleUse(vk::CommandPool CommandPool, vk::Queue Queue)
{
    End();

    vk::SubmitInfo SubmitInfo {};
    SubmitInfo.setCommandBufferCount(1)
              .setPCommandBuffers(&Handle);

    Queue.submit({ SubmitInfo });
    Queue.waitIdle();
    
    Free();
}