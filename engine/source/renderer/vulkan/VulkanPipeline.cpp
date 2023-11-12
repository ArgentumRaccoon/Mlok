#include "VulkanPipeline.h"

#include "VulkanContext.h"
#include "VulkanRenderPass.h"
#include "VulkanCommandBuffer.h"
#include "VulkanUtils.h"

#include "core/Logger.h"

#include "math/MathTypes.h"


VulkanPipeline::VulkanPipeline(VulkanContext* inContext,
                               VulkanRenderPass* RenderPass,
                               std::vector<vk::VertexInputAttributeDescription>& Attributes,
                               std::vector<vk::DescriptorSetLayout>& DescriptorSetLayouts,
                               std::vector<vk::PipelineShaderStageCreateInfo>& Stages,
                               vk::Viewport Viewport,
                               vk::Rect2D Scissor,
                               bool bIsWireframe)
    : Context { inContext }
{
    Create(Context, 
           RenderPass,
           Attributes, 
           DescriptorSetLayouts,
           Stages, 
           Viewport, 
           Scissor, 
           bIsWireframe);
}

VulkanPipeline::~VulkanPipeline()
{
    Destroy();
}

bool VulkanPipeline::Create(VulkanContext* inContext,
                            VulkanRenderPass* RenderPass,
                            std::vector<vk::VertexInputAttributeDescription>& Attributes,
                            std::vector<vk::DescriptorSetLayout>& DescriptorSetLayouts,
                            std::vector<vk::PipelineShaderStageCreateInfo>& Stages,
                            vk::Viewport Viewport,
                            vk::Rect2D Scissor,
                            bool bIsWireframe)
{
    if (Context != inContext)
    {
        Context = inContext;
    }

    vk::PipelineViewportStateCreateInfo ViewportStateCreateInfo {};
    ViewportStateCreateInfo.setViewportCount(1)
                           .setPViewports(&Viewport)
                           .setScissorCount(1)
                           .setPScissors(&Scissor);

    vk::PipelineRasterizationStateCreateInfo RasterizerCreateInfo {};
    RasterizerCreateInfo.setDepthClampEnable(false)
                        .setRasterizerDiscardEnable(false)
                        .setPolygonMode(bIsWireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill)
                        .setLineWidth(1.f)
                        .setCullMode(vk::CullModeFlagBits::eBack)
                        .setFrontFace(vk::FrontFace::eCounterClockwise)
                        .setDepthBiasEnable(false)
                        .setDepthBiasConstantFactor(0.f)
                        .setDepthBiasClamp(false)
                        .setDepthBiasSlopeFactor(0.f);

    vk::PipelineMultisampleStateCreateInfo MultisamplingCreateInfo {};
    MultisamplingCreateInfo.setSampleShadingEnable(false)
                           .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                           .setMinSampleShading(1.f)
                           .setPSampleMask(nullptr)
                           .setAlphaToCoverageEnable(false)
                           .setAlphaToOneEnable(false);

    vk::PipelineDepthStencilStateCreateInfo DepthStencilCreateInfo {};
    DepthStencilCreateInfo.setDepthTestEnable(true)
                          .setDepthWriteEnable(true)
                          .setDepthCompareOp(vk::CompareOp::eLess)
                          .setDepthBoundsTestEnable(false)
                          .setStencilTestEnable(false);

    vk::PipelineColorBlendAttachmentState ColorBlendAttachmentState {};
    ColorBlendAttachmentState.setBlendEnable(true)
                             .setSrcColorBlendFactor(vk::BlendFactor::eSrcColor)
                             .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcColor)
                             .setColorBlendOp(vk::BlendOp::eAdd)
                             .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
                             .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                             .setAlphaBlendOp(vk::BlendOp::eAdd)
                             .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    const std::vector<vk::PipelineColorBlendAttachmentState> ColorBlendAttachmentStatesList { ColorBlendAttachmentState };

    vk::PipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo {};
    ColorBlendStateCreateInfo.setLogicOpEnable(false)
                             .setLogicOp(vk::LogicOp::eCopy)
                             .setAttachments(ColorBlendAttachmentStatesList);

    const std::vector<vk::DynamicState> DynamicStates { vk::DynamicState::eViewport,
                                                        vk::DynamicState::eScissor,
                                                        vk::DynamicState::eLineWidth };
    vk::PipelineDynamicStateCreateInfo DynamicStateCreateInfo {};
    DynamicStateCreateInfo.setDynamicStates(DynamicStates);

    vk::VertexInputBindingDescription BindingDescription {};
    BindingDescription.setBinding(0)
                      .setStride(sizeof(Vertex3D))
                      .setInputRate(vk::VertexInputRate::eVertex);

    std::vector<vk::VertexInputBindingDescription> BindingDescriptionsList { BindingDescription };

    vk::PipelineVertexInputStateCreateInfo VertexInputStateCreateInfo {};
    VertexInputStateCreateInfo.setVertexBindingDescriptions(BindingDescriptionsList)
                              .setVertexAttributeDescriptions(Attributes);

    vk::PipelineInputAssemblyStateCreateInfo InputAssembly {};
    InputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList)
                 .setPrimitiveRestartEnable(false);

    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo {};
    PipelineLayoutCreateInfo.setSetLayouts(DescriptorSetLayouts);

    {
        const auto& CreateResult = Context->pDevice->LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo, Context->Allocator);
        if (!VulkanUtils::ResultIsSuccess(CreateResult.result))
        {
            MlokFatal("Failed to create Vulkan Pipeline Layout: %s", VulkanUtils::VulkanResultString(CreateResult.result, true).c_str());
            return false;
        }

        Layout = CreateResult.value;
    }

    vk::GraphicsPipelineCreateInfo PipelineCreateInfo {};
    PipelineCreateInfo.setStages(Stages)
                      .setPVertexInputState(&VertexInputStateCreateInfo)
                      .setPInputAssemblyState(&InputAssembly)
                      .setPViewportState(&ViewportStateCreateInfo)
                      .setPRasterizationState(&RasterizerCreateInfo)
                      .setPMultisampleState(&MultisamplingCreateInfo)
                      .setPDepthStencilState(&DepthStencilCreateInfo)
                      .setPColorBlendState(&ColorBlendStateCreateInfo)
                      .setPDynamicState(&DynamicStateCreateInfo)
                      .setPTessellationState(nullptr)
                      .setLayout(Layout)
                      .setRenderPass(RenderPass->Get() ? *RenderPass->Get() : nullptr)
                      .setSubpass(0)
                      .setBasePipelineHandle(nullptr)
                      .setBasePipelineIndex(-1);

    {
        const auto& CreateResult = Context->pDevice->LogicalDevice.createGraphicsPipeline(nullptr, PipelineCreateInfo, Context->Allocator);
        if (!VulkanUtils::ResultIsSuccess(CreateResult.result))
        {
            MlokFatal("Failed to create Vulkan Pipeline: %s", VulkanUtils::VulkanResultString(CreateResult.result, true).c_str());
            return false;
        }

        Handle = CreateResult.value;
    }

    return true;
}

void VulkanPipeline::Destroy()
{
    if (Handle)
    {
        Context->pDevice->LogicalDevice.destroyPipeline(Handle, Context->Allocator);
        Handle = nullptr;
    }

    if (Layout)
    {
        Context->pDevice->LogicalDevice.destroyPipelineLayout(Layout, Context->Allocator);
        Layout = nullptr;
    }
}

void VulkanPipeline::Bind(VulkanCommandBuffer* CommandBuffer, vk::PipelineBindPoint BindPoint)
{
    if (CommandBuffer && CommandBuffer->Get())
    {
        CommandBuffer->Get()->bindPipeline(BindPoint, Handle);
    }
}