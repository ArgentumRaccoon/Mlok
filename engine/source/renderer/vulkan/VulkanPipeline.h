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

        vk::Pipeline* Get() { return &Handle; }

    private:
        VulkanContext* Context; // Cached pointer to backend context
        
        vk::Pipeline Handle;
        vk::PipelineLayout Layout;
};