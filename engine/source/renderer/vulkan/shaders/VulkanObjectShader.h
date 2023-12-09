#pragma once

#include "renderer/vulkan/VulkanTypes.inl"
#include "renderer/vulkan/VulkanPipeline.h"
#include "renderer/vulkan/VulkanBuffer.h"
#include "renderer/RendererTypes.inl"

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
        
        void UpdateGlobalState();

        GlobalUniformObject& GetGlobalUBO() { return GlobalUBO; };

    private:
        VulkanContext* Context; // Cached pointer to backend context

        std::array<VulkanShaderStage, OBJECT_SHADER_STAGE_COUNT> Stages;

        vk::DescriptorPool GlobalDescriptorPool;
        vk::DescriptorSetLayout GlobalDescriptorSetLayout;
        vk::DescriptorSet GlobalDescriptorSets[3]; // One per each frame rendering

        GlobalUniformObject GlobalUBO;

        VulkanBuffer GlobalUniformBuffer;
        
        VulkanPipeline Pipeline;
};