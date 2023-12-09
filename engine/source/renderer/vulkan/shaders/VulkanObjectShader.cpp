#include "VulkanObjectShader.h"

#include "renderer/vulkan/VulkanContext.h"
#include "renderer/vulkan/VulkanUtils.h"

#include "core/MlokUtils.h"
#include "core/Logger.h"
#include "math/MathTypes.h"

#include "platform/FileSystem.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

VulkanShaderStage::VulkanShaderStage(VulkanContext* inContext,
                                     std::string Name,
                                     std::string TypeStr,
                                     vk::ShaderStageFlagBits ShaderStageFlag)
    : Context { inContext }
{
    Create(Context, Name, TypeStr, ShaderStageFlag);
}

VulkanShaderStage::~VulkanShaderStage()
{
    Destroy();
}

bool VulkanShaderStage::Create(VulkanContext* inContext,
                               std::string Name,
                               std::string TypeStr,
                               vk::ShaderStageFlagBits ShaderStageFlag)
{
    if (Context != inContext)
    {
        Context = inContext;
    }

    std::string Filename = MlokUtils::StringFormat("assets/shaders/%s.%s.spv", Name.c_str(), TypeStr.c_str());

    FileHandle File { Filename };
    if (!File.Open(true))
    {
        MlokError("Unable to read shader module: %s", Filename.c_str());
        return false;
    }

    size_t FileSize = 0;
    std::vector<char> FileBuffer; // TODO: custom allocator
    if (!File.ReadAllBytes(FileBuffer, &FileSize))
    {
        MlokError("Unable to read shader module: %s", Filename.c_str());
        return false;
    }

    CreateInfo = vk::ShaderModuleCreateInfo {};
    CreateInfo.setCodeSize(FileSize)
              .setPCode(reinterpret_cast<uint32_t*>(FileBuffer.data()));

    const auto& CreateResult = Context->pDevice->LogicalDevice.createShaderModule(CreateInfo, Context->Allocator);
    if (!VulkanUtils::ResultIsSuccess(CreateResult.result))
    {
        MlokFatal("Failed to create Vulkan Shader Module: %s", VulkanUtils::VulkanResultString(CreateResult.result, true).c_str());
        return false;
    }

    Handle = CreateResult.value;

    StageCreateInfo = vk::PipelineShaderStageCreateInfo {};
    StageCreateInfo.setStage(ShaderStageFlag)
                   .setModule(Handle)
                   .setPName("main");

    return true;                   
}

void VulkanShaderStage::Destroy()
{
    if (Handle)
    {
        Context->pDevice->LogicalDevice.destroyShaderModule(Handle, Context->Allocator);
        Handle = nullptr;
    }
}

VulkanObjectShader::VulkanObjectShader(VulkanContext* inContext)
    : Context { inContext }
{
    Create(Context);
}

VulkanObjectShader::~VulkanObjectShader()
{
    Destroy();
}

bool VulkanObjectShader::Create(VulkanContext* inContext)
{
    if (Context != inContext)
    {
        Context = inContext;
    }

    std::string StageTypeStrs[OBJECT_SHADER_STAGE_COUNT] = { "vert", "frag" };
    vk::ShaderStageFlagBits StageTypes[OBJECT_SHADER_STAGE_COUNT] = { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment };

    for (uint32_t i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i)
    {
        if (!Stages[i].Create(Context, BUILTIN_SHADER_NAME_OBJECT, StageTypeStrs[i], StageTypes[i]))
        {
            MlokError("Unable to create %s shader module for '%s'", StageTypeStrs[i].c_str(), BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    vk::DescriptorSetLayoutBinding GlobalUBOLayoutBinding {};
    GlobalUBOLayoutBinding.setBinding(0)
                          .setDescriptorCount(1)
                          .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                          .setImmutableSamplers({})
                          .setStageFlags(vk::ShaderStageFlagBits::eVertex);

    vk::DescriptorSetLayoutCreateInfo GlobalLayoutInfo {};
    GlobalLayoutInfo.setBindingCount(1)
                    .setPBindings(&GlobalUBOLayoutBinding);
    {
        auto Result = Context->pDevice->LogicalDevice.createDescriptorSetLayout(GlobalLayoutInfo, Context->Allocator);
        if (!VulkanUtils::ResultIsSuccess(Result.result))
        {
            MlokError("Error creating Global Descriptor Layout: %s", VulkanUtils::VulkanResultString(Result.result, true).c_str());
            return false;
        }
        GlobalDescriptorSetLayout = Result.value;
    }

    vk::DescriptorPoolSize GlobalPoolSize {};
    GlobalPoolSize.setType(vk::DescriptorType::eUniformBuffer)
                  .setDescriptorCount(Context->pSwapchain->GetImageCount());

    vk::DescriptorPoolCreateInfo GlobalPoolInfo {};
    GlobalPoolInfo.setPoolSizeCount(1)
                  .setPPoolSizes(&GlobalPoolSize)
                  .setMaxSets(Context->pSwapchain->GetImageCount());
    {
        auto Result = Context->pDevice->LogicalDevice.createDescriptorPool(GlobalPoolInfo, Context->Allocator);
        if (!VulkanUtils::ResultIsSuccess(Result.result))
        {
            MlokError("Error creating Global Descriptor Pool: %s", VulkanUtils::VulkanResultString(Result.result, true).c_str());
            return false;
        }
        GlobalDescriptorPool = Result.value;
    }

    vk::Viewport Viewport;
    Viewport.setX(0.f)
            .setY(static_cast<float>(Context->FramebufferHeight))
            .setWidth(static_cast<float>(Context->FramebufferWidth))
            .setHeight(static_cast<float>(Context->FramebufferHeight))
            .setMinDepth(0.f)
            .setMaxDepth(1.f);
    
    vk::Rect2D Scissor;
    Scissor.setOffset({ 0, 0 })
           .setExtent({ Context->FramebufferWidth, Context->FramebufferHeight });

    const size_t AttributeCount = 1;
    std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions(AttributeCount);
    std::vector<vk::Format> Formats(AttributeCount);
    Formats[0] = vk::Format::eR32G32B32Sfloat;
    std::vector<uint64_t> Sizes(AttributeCount);
    Sizes[0] = sizeof(Vec3);

    uint32_t Offset = 0;

    for (size_t i = 0; i < AttributeCount; ++i)
    {
        AttributeDescriptions[i].setBinding(0)
                                .setLocation(static_cast<uint32_t>(i))
                                .setFormat(Formats[i])
                                .setOffset(Offset);
        Offset += Sizes[i];
    }

    std::vector<vk::DescriptorSetLayout> DescriptorSetLayouts = { GlobalDescriptorSetLayout };

    std::vector<vk::PipelineShaderStageCreateInfo> StageCreateInfos(OBJECT_SHADER_STAGE_COUNT);
    for (size_t i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i)
    {
        StageCreateInfos[i] = Stages[i].StageCreateInfo;
    }

    const bool bIsWireframe = false;
    if (!Pipeline.Create(Context,
                         Context->pMainRenderPass.get(),
                         AttributeDescriptions,
                         DescriptorSetLayouts,
                         StageCreateInfos,
                         Viewport,
                         Scissor,
                         bIsWireframe))
    {
        MlokError("Failed to load graphics pipeline for object shader");
        return false;
    }

    if (!GlobalUniformBuffer.Create(Context, 
                                    sizeof(GlobalUniformObject),
                                    vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
                                    true))
    {
        MlokError("Global Uniform Buffer creation failed for object shader");
        return false;
    }

    std::vector<vk::DescriptorSetLayout> GlobalLayouts { 3, GlobalDescriptorSetLayout };

    vk::DescriptorSetAllocateInfo DesciptorSetAllocateInfo {};
    DesciptorSetAllocateInfo.setDescriptorPool(GlobalDescriptorPool)
                            .setSetLayouts(GlobalLayouts);

    {
        auto Result = Context->pDevice->LogicalDevice.allocateDescriptorSets(DesciptorSetAllocateInfo);
        if (!VulkanUtils::ResultIsSuccess(Result.result))
        {
            MlokError("Error allocating Global DescriptorSets for object shader: %s", VulkanUtils::VulkanResultString(Result.result, true).c_str());
            return false;
        }
        for (size_t i = 0; i < Result.value.size(); ++i)
        {
            GlobalDescriptorSets[i] = Result.value[i];
        }
    }

    return true;
}

void VulkanObjectShader::Destroy()
{
    GlobalUniformBuffer.Destroy();

    Pipeline.Destroy();

    Context->pDevice->LogicalDevice.destroyDescriptorPool(GlobalDescriptorPool, Context->Allocator);

    Context->pDevice->LogicalDevice.destroyDescriptorSetLayout(GlobalDescriptorSetLayout, Context->Allocator);

    for (auto& Stage : Stages)
    {
        Stage.Destroy();
    }
}

void VulkanObjectShader::Use()
{
    uint32_t ImageIndex = Context->ImageIndex;
    Pipeline.Bind(&Context->GraphicsCommandBuffers[ImageIndex], vk::PipelineBindPoint::eGraphics);
}

void VulkanObjectShader::UpdateGlobalState()
{
    auto ImageIndex = Context->ImageIndex;
    auto CommandBuffer = Context->GraphicsCommandBuffers[ImageIndex].Get();
    auto GlobalDescriptor = GlobalDescriptorSets[ImageIndex];

    CommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Pipeline.GetLayout(), 0, 1, &GlobalDescriptor, 0, nullptr);

    const uint32_t Range = static_cast<uint32_t>(sizeof(GlobalUniformObject));
    const uint64_t Offset = 0;
    
    GlobalUniformBuffer.LoadData(Offset, Range, {}, &GlobalUBO);

    vk::DescriptorBufferInfo BufferInfo {};
    BufferInfo.setBuffer(*GlobalUniformBuffer.Get())
              .setOffset(Offset)
              .setRange(Range);

    vk::WriteDescriptorSet DescriptorSetWrite {};
    DescriptorSetWrite.setDstSet(GlobalDescriptor)
                      .setDstBinding(0)
                      .setDstArrayElement(0)
                      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                      .setDescriptorCount(1)
                      .setPBufferInfo(&BufferInfo);

    Context->pDevice->LogicalDevice.updateDescriptorSets(1, &DescriptorSetWrite, 0, nullptr);
}