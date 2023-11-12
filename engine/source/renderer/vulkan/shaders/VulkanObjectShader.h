#pragma once

#include "renderer/vulkan/VulkanTypes.inl"
#include "renderer/vulkan/VulkanPipeline.h"

#include <array>

class VulkanContext;

class VulkanShaderStage
{
    public:
        VulkanShaderStage() = default;
        VulkanShaderStage(VulkanContext* Context,
                          std::string Name,
                          std::string TypeStr,
                          vk::ShaderStageFlagBits ShaderStageFlag);
        ~VulkanShaderStage();

        vk::ShaderModule* Get() { return &Handle; }

        bool Create(VulkanContext* Context,
                    std::string Name,
                    std::string TypeStr,
                    vk::ShaderStageFlagBits ShaderStageFlag);
        void Destroy();

        vk::ShaderModuleCreateInfo CreateInfo;
        vk::PipelineShaderStageCreateInfo StageCreateInfo;

    private:
        VulkanContext* Context; // Cached pointer to backend context

        vk::ShaderModule Handle;
};

class VulkanObjectShader
{
    public:
        VulkanObjectShader() = default;
        VulkanObjectShader(VulkanContext* inContext);
        ~VulkanObjectShader();

        bool Create(VulkanContext* inContext);
        void Destroy();

        void Use();

    private:
        VulkanContext* Context; // Cached pointer to backend context

        std::array<VulkanShaderStage, OBJECT_SHADER_STAGE_COUNT> Stages;
        VulkanPipeline Pipeline;
};