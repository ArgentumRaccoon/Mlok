#pragma once

#include "VulkanTypes.inl"

class VulkanContext;
class VulkanRenderPass;
class VulkanCommandBuffer;

class VulkanPipeline
{
    public:
        VulkanPipeline() = default;
        VulkanPipeline(VulkanContext* inContext,
                       VulkanRenderPass* RenderPass,
                       std::vector<vk::VertexInputAttributeDescription>& Attributes,
                       std::vector<vk::DescriptorSetLayout>& DescriptorSetLayouts,
                       std::vector<vk::PipelineShaderStageCreateInfo>& Stages,
                       vk::Viewport Viewport,
                       vk::Rect2D Scissor,
                       bool bIsWireframe);
        VulkanPipeline(const VulkanPipeline& Other) = delete;
        VulkanPipeline& operator=(const VulkanPipeline& Other) = delete;
        ~VulkanPipeline();

        vk::Pipeline* Get() { return &Handle; }

        vk::PipelineLayout GetLayout() { return Layout; }

        bool Create(VulkanContext* inContext,
                    VulkanRenderPass* RenderPass,
                    std::vector<vk::VertexInputAttributeDescription>& Attributes,
                    std::vector<vk::DescriptorSetLayout>& DescriptorSetLayouts,
                    std::vector<vk::PipelineShaderStageCreateInfo>& Stages,
                    vk::Viewport Viewport,
                    vk::Rect2D Scissor,
                    bool bIsWireframe);
        void Destroy();

        void Bind(VulkanCommandBuffer* CommandBuffer, vk::PipelineBindPoint BindPoint);

    private:
        VulkanContext* Context; // Cached pointer to backend context
        
        vk::Pipeline Handle;
        vk::PipelineLayout Layout;
};