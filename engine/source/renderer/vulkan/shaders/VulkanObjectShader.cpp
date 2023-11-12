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

    std::vector<vk::PipelineShaderStageCreateInfo> StageCreateInfos(OBJECT_SHADER_STAGE_COUNT);
    for (size_t i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i)
    {
        StageCreateInfos[i] = Stages[i].StageCreateInfo;
    }

    const bool bIsWireframe = false;
    std::vector<vk::DescriptorSetLayout> DescriptorSetLayouts(AttributeCount);
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

    return true;
}

void VulkanObjectShader::Destroy()
{
    Pipeline.Destroy();

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