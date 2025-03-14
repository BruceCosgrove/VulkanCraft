#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include <vulkan/vulkan.h>
#include <filesystem>
#include <memory>
#include <span>
#include <vector>

namespace eng
{
    class RenderContext;
    class UniformBuffer;
    class StorageBuffer;

    struct ShaderVertexBufferBinding
    {
        std::uint32_t Binding = 0;
        VkVertexInputRate InputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        std::vector<std::uint32_t> Locations;
    };

    struct ShaderUniformBufferBinding
    {
        std::uint32_t Binding = 0;
        std::shared_ptr<UniformBuffer> Descriptor;
    };

    struct ShaderStorageBufferBinding
    {
        std::uint32_t Binding = 0;
        std::shared_ptr<StorageBuffer> Descriptor;
    };

    struct ShaderSamplerBinding
    {
        std::uint32_t Binding = 0;
        VkSampler Sampler = VK_NULL_HANDLE;
        VkImageView ImageView = VK_NULL_HANDLE;
    };

    struct ShaderDescriptorSetData
    {
        std::span<ShaderUniformBufferBinding> UniformBuffers;
        std::span<ShaderStorageBufferBinding> StorageBuffers;
        std::span<ShaderSamplerBinding> Samplers;
    };

    struct ShaderInfo
    {
        RenderContext* RenderContext = nullptr;
        std::filesystem::path Filepath;
        std::span<ShaderVertexBufferBinding> VertexBufferBindings;
        VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkRenderPass RenderPass = VK_NULL_HANDLE;
    };

    // TODO: separate shader from pipeline.
    // The pipeline can be recreated without recreating the shader.
    class Shader
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Shader);

        Shader(ShaderInfo const& info);
        ~Shader();

        void Bind(VkCommandBuffer commandBuffer);
        void UpdateDescriptorSet(ShaderDescriptorSetData const& data);
    private:
        std::vector<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> CompileExistingSources(
            std::filesystem::path const& filepath
        );

        std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageInfos(
            std::span<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> stages
        );

        void Reflect(
            ShaderInfo const& info,
            std::span<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> stages,
            std::vector<std::uint32_t>& strides,
            std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions,
            std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings,
            std::vector<VkDescriptorPoolSize>& descriptorPoolSizes
        );

        void CreateDescriptorSetLayout(std::span<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings);
        void CreateDescriptorPool(std::span<VkDescriptorPoolSize> descriptorPoolSizes);
        void CreateDescriptorSets();
        void CreatePipelineLayout();

        void CreatePipeline(
            ShaderInfo const& info,
            std::span<std::uint32_t> strides,
            std::span<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions,
            std::span<VkPipelineShaderStageCreateInfo> pipelineShaderStageInfos
        );
    private:
        RenderContext& m_Context; // non-owning

        VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        // NOTE: Assuming one descriptor set per frame.
        // Realistically, there can/should be up to four,
        // but this is the simplest option right now.
        std::vector<VkDescriptorSet> m_DescriptorSets;

        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
    };
}
