#include "Shader.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/IO/FileIO.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include "Engine/Rendering/UniformBuffer.hpp"
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_reflect.hpp>
#include <array>
#include <ranges>

namespace fs = std::filesystem;

namespace eng
{
    static VkFormat GetVulkanFormat(spirv_cross::SPIRType::BaseType baseType, std::uint32_t count)
    {
        ENG_ASSERT(
            baseType == spirv_cross::SPIRType::Int or
            baseType == spirv_cross::SPIRType::UInt or
            baseType == spirv_cross::SPIRType::Float or
            baseType == spirv_cross::SPIRType::Double
        );
        ENG_ASSERT(0 < count and count <= 4);

        switch (baseType)
        {
            case spirv_cross::SPIRType::Int:    return static_cast<VkFormat>(VK_FORMAT_R32_SINT   + 3 * (count - 1));
            case spirv_cross::SPIRType::UInt:   return static_cast<VkFormat>(VK_FORMAT_R32_UINT   + 3 * (count - 1));
            case spirv_cross::SPIRType::Float:  return static_cast<VkFormat>(VK_FORMAT_R32_SFLOAT + 3 * (count - 1));
            case spirv_cross::SPIRType::Double: return static_cast<VkFormat>(VK_FORMAT_R64_SFLOAT + 3 * (count - 1));
        }

        return VK_FORMAT_MAX_ENUM;
    }

    Shader::Shader(ShaderInfo const& info)
        : m_Context(*info.RenderContext)
    {
        VkDevice device = m_Context.GetDevice();
        std::uint32_t swapchainImageCount = m_Context.GetSwapchainImageCount();

        m_FrameData.resize(swapchainImageCount);

        auto stages = CompileExistingSources(info.Filepath);
        auto pipelineShaderStageInfos = GetPipelineShaderStageInfos(stages);

        // Get vertex stride, vertex input attribute descriptions, descriptor set layout bindings, and descriptor pool sizes.
        std::uint32_t stride = 0;
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
        std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
        Reflect(stages, stride, vertexInputAttributeDescriptions, descriptorSetLayoutBindings, descriptorPoolSizes);

        CreateDescriptorSetLayout(descriptorSetLayoutBindings);
        CreateDescriptorPool(descriptorPoolSizes);
        CreateDescriptorSets();

        auto dynamicStates = std::to_array({
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        });

        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
        dynamicStateInfo.pDynamicStates = dynamicStates.data();

        // TODO: read from shader reflection
        // TODO: this assumes all attributes are per vertex, and not per instance.
        VkVertexInputBindingDescription vertexInputBindingDescription{};
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = stride;
        vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
        vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        // TODO: these are per vertex buffer bound to the pipeline.
        vertexInputStateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertexInputAttributeDescriptions.size());
        vertexInputStateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo{};
        inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // TODO: relevant
        inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo pipelineViewportStateInfo{};
        pipelineViewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipelineViewportStateInfo.viewportCount = 1;
        pipelineViewportStateInfo.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizationStateInfo{};
        rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL; // TODO: relevant
        rasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // TODO: relevant
        rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationStateInfo.depthBiasEnable = VK_FALSE; // TODO: some shadow implementations apparently use this?
        rasterizationStateInfo.lineWidth = 1.0f;

        // TODO: look into this if aliasing is an issue.
        VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
        multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateInfo.sampleShadingEnable = VK_FALSE;

        //VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
        //depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        //depthStencilInfo.depthTestEnable = VK_FALSE; // TODO: relevant
        //depthStencilInfo.depthWriteEnable = VK_FALSE; // TODO: relevant
        //depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS; // TODO: relevant

        // NOTE: one of these per attachments, e.g. color outputs from fragment shader.
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
        colorBlendAttachmentState.blendEnable = VK_TRUE;
        colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
        colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateInfo.attachmentCount = 1;
        colorBlendStateInfo.pAttachments = &colorBlendAttachmentState;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; // TODO: descriptor pool
        pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout; // TODO: descriptor pool
        // NOTE: this is where push constants would go.

        VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout.");

        VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
        graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineInfo.stageCount = static_cast<std::uint32_t>(pipelineShaderStageInfos.size());
        graphicsPipelineInfo.pStages = pipelineShaderStageInfos.data();
        graphicsPipelineInfo.pVertexInputState = &vertexInputStateInfo;
        graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
        graphicsPipelineInfo.pTessellationState = nullptr; // TODO
        graphicsPipelineInfo.pViewportState = &pipelineViewportStateInfo;
        graphicsPipelineInfo.pRasterizationState = &rasterizationStateInfo;
        graphicsPipelineInfo.pMultisampleState = &multisampleStateInfo;
        graphicsPipelineInfo.pDepthStencilState = nullptr; // TODO &depthStencilInfo;
        graphicsPipelineInfo.pColorBlendState = &colorBlendStateInfo;
        graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;
        graphicsPipelineInfo.layout = m_PipelineLayout;
        graphicsPipelineInfo.renderPass = info.RenderPass;
        graphicsPipelineInfo.subpass = 0;

        result = vkCreateGraphicsPipelines(device, nullptr, 1, &graphicsPipelineInfo, nullptr, &m_Pipeline);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create graphics pipeline.");

        // Destroy the temporary modules.
        for (auto& pipelineShaderStageInfo : pipelineShaderStageInfos)
            vkDestroyShaderModule(device, pipelineShaderStageInfo.module, nullptr);
    }

    Shader::~Shader()
    {
        VkDevice device = m_Context.GetDevice();

        vkDestroyPipeline(device, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
        vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
    }

    void Shader::Bind(VkCommandBuffer commandBuffer)
    {
        std::uint32_t swapchainImageIndex = m_Context.GetSwapchainImageIndex();
        auto& descriptorSet = m_FrameData[swapchainImageIndex].DescriptorSet;

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

        // TODO: more complex systems with multiple bound sets based on usage frequency.
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout,
            0,
            1,
            &descriptorSet,
            // TODO: dynamic offsets
            0,
            nullptr
        );
    }

    void Shader::UpdateDescriptorSet()
    {
        std::uint32_t swapchainImageIndex = m_Context.GetSwapchainImageIndex();

        // Get the data for this frame.
        auto& descriptorSet = m_FrameData[swapchainImageIndex].DescriptorSet;
        auto& uniformBuffers = m_FrameData[swapchainImageIndex].UniformBuffers;

        // Count the number of descriptor sets to update.
        std::uint32_t uniformBufferCount = static_cast<std::uint32_t>(uniformBuffers.size());

        // TODO: other resource types, e.g. storage buffers
        std::uint32_t writeCount = uniformBufferCount;
        if (writeCount == 0)
            return;

        // TODO: don't allocate every frame.
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(writeCount);
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(uniformBufferCount);

        for (auto& uniformBuffer : uniformBuffers)
        {
            auto& bufferInfo = bufferInfos.emplace_back();
            bufferInfo.buffer = uniformBuffer.Resource->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;

            // TODO: multiple VkWriteDescriptorSet's, same dstSet, different dstBinding, different descriptorType
            // e.g. VkWriteDescriptorSet 0: dstSet = set 0, dstBinding = 0, descriptorType = UNIFORM_BUFFER
            // e.g. VkWriteDescriptorSet 1: dstSet = set 0, dstBinding = 1, descriptorType = STORAGE_BUFFER
            // e.g. VkWriteDescriptorSet 2: dstSet = set 0, dstBinding = 2, descriptorType = SAMPLER

            auto& write = writes.emplace_back();
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSet;
            write.dstBinding = uniformBuffer.Binding;
            write.dstArrayElement = 0; // TODO: arrays
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // TODO: other types
            write.descriptorCount = 1; // TODO: arrays
            //write.pImageInfo = // TODO: samplers
            write.pBufferInfo = &bufferInfo;
            //write.pTexelBufferView = // TODO: idk what these are
        }

        vkUpdateDescriptorSets(m_Context.GetDevice(), writeCount, writes.data(), 0, nullptr);
    }

    UniformBuffer* Shader::GetUniformBuffer(std::uint32_t binding)
    {
        auto& uniformBuffers = m_FrameData[m_Context.GetSwapchainImageIndex()].UniformBuffers;
        for (auto& uniformBuffer : uniformBuffers)
            if (uniformBuffer.Binding == binding)
                return uniformBuffer.Resource.get();

        ENG_ASSERT(false);
        return nullptr;
    }

    std::vector<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> Shader::CompileExistingSources(fs::path const& filepath)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        return // NOTE: Yes, no semicolon. This is returning the following pipes' results.

        // For all the supported shader stages...
        std::to_array<std::tuple<std::string_view, shaderc_shader_kind, VkShaderStageFlagBits>>
        ({
            { ".vert.glsl", shaderc_glsl_vertex_shader,          VK_SHADER_STAGE_VERTEX_BIT                  },
            { ".tcon.glsl", shaderc_glsl_tess_control_shader,    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT    },
            { ".teva.glsl", shaderc_glsl_tess_evaluation_shader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT },
            { ".geom.glsl", shaderc_glsl_geometry_shader,        VK_SHADER_STAGE_GEOMETRY_BIT                },
            { ".frag.glsl", shaderc_glsl_fragment_shader,        VK_SHADER_STAGE_FRAGMENT_BIT                },
            { ".comp.glsl", shaderc_glsl_compute_shader,         VK_SHADER_STAGE_COMPUTE_BIT                 },
        }) |

        // Get all files at the given filepath with shader extensions.
        std::views::transform([&](auto&& stage)
        {
            return std::make_tuple(
                fs::path(filepath).replace_extension(std::get<0>(stage)),
                std::get<1>(stage),
                std::get<2>(stage)
            );
        }) |

        // Keep processing only the files that exist.
        std::views::filter([&](auto&& stage)
        {
            std::error_code error;
            return fs::exists(std::get<0>(stage), error);
        }) |

        // Compile present source files.
        std::views::transform([&](auto&& stage)
        {
            fs::path const& stageFilepath = std::get<0>(stage);
            shaderc_shader_kind kind = std::get<1>(stage);

            // Read the stage's source file.
            std::string contents;
            ENG_VERIFY(ReadFile(stageFilepath, contents), "Failed to read shader source file.");

            // Compile the stage.
            shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(contents, kind, stageFilepath.string().c_str(), options);
            ENG_ASSERT(
                result.GetCompilationStatus() == shaderc_compilation_status_success,
                "Failed to compile shader:\n{}", result.GetErrorMessage()
            );

            // Return the spir-v bytecode and the vulkan stage flags.
            return std::make_tuple(
                std::vector((std::uint8_t*)result.begin(), (std::uint8_t*)result.end()),
                std::get<2>(stage)
            );
        }) |

        // Collate the existing shader stages.
        std::ranges::to<std::vector>();
    }

    std::vector<VkPipelineShaderStageCreateInfo> Shader::GetPipelineShaderStageInfos(std::span<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> stages)
    {
        VkDevice device = m_Context.GetDevice();

        return // NOTE: Yes, no semicolon. This is returning the following pipes' results.

        stages |

        // Create shader modules.
        std::views::transform([&](auto&& stage)
        {
            std::vector<std::uint8_t> const& code = std::get<0>(stage);

            VkShaderModuleCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            info.codeSize = static_cast<std::uint32_t>(code.size());
            info.pCode = (std::uint32_t const*)code.data();

            VkShaderModule shaderModule = VK_NULL_HANDLE;
            VkResult result = vkCreateShaderModule(device, &info, nullptr, &shaderModule);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create shader module.");

            return std::make_tuple(shaderModule, std::get<1>(stage));
        }) |

        // Extract the stage flag bits and setup some create infos.
        std::views::transform([&](auto&& stage)
        {
            VkPipelineShaderStageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            info.stage = std::get<1>(stage);
            info.module = std::get<0>(stage);
            info.pName = "main";
            return info;
        }) |

        // Collate the create infos into a vector.
        std::ranges::to<std::vector>();
    }

    void Shader::Reflect(
        std::span<std::tuple<std::vector<std::uint8_t>, VkShaderStageFlagBits>> stages,
        std::uint32_t& stride,
        std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions,
        std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings,
        std::vector<VkDescriptorPoolSize>& descriptorPoolSizes
    )
    {
        // The first stage requires extra reflection for the
        // vertex stride and input attribute descriptions.

        std::vector<std::uint8_t> const& code = std::get<0>(stages.front());
        // NOTE: These two are so large that they have to be stored on the heap (~18KB combined).
        auto reflection = std::make_unique<spirv_cross::CompilerReflection>((std::uint32_t const*)code.data(), code.size() / sizeof(std::uint32_t));
        auto resources = std::make_unique<spirv_cross::ShaderResources>(reflection->get_shader_resources());

        ENG_ASSERT(std::get<1>(stages.front()) != VK_SHADER_STAGE_COMPUTE_BIT, "TODO: how to support compute shaders.");
        //reflection->get_work_group_size_specialization_constants(); // related to compute shaders

        // Do extra processing for the vertex shader.
        // TODO: this assumes all attributes are per vertex, and not per instance.
        vertexInputAttributeDescriptions.reserve(resources->stage_inputs.size());
        for (std::uint32_t& offset = stride; auto& resource : resources->stage_inputs)
        {
            auto& type = reflection->get_type_from_variable(resource.id);

            std::uint32_t size = type.width / 8; // byte size per component
            std::uint32_t count = type.vecsize; // component count
            // NOTE: yes, this will underflow the intermediate uint32, which is well-defined behavior.
            offset += (size - offset) % size; // alignment padding between the last attribute and this one

            auto& description = vertexInputAttributeDescriptions.emplace_back();
            description.location = reflection->get_decoration(resource.id, spv::DecorationLocation);
            description.binding = 0;
            description.format = GetVulkanFormat(type.basetype, count);
            description.offset = offset;

            offset += size * count; // advance the offset to after this attribute
        }

        // The rare nested lambdas, ~oooOOOo~ooOOO~ooOo~OOO~!
        auto processStage = [&](VkShaderStageFlagBits stage)
        {
            auto processResources = [&](
                spirv_cross::SmallVector<spirv_cross::Resource>& resources,
                VkDescriptorType descriptorType,
                auto&& specializationFunc)
            {
                for (auto& resource : resources)
                {
                    auto& type = reflection->get_type_from_variable(resource.id);
                    std::uint32_t binding = reflection->get_decoration(resource.id, spv::DecorationBinding);
                    std::uint32_t count = type.array.size() == 0 ? 1 : type.array[0];

                    // Get the descriptor set layout binding if it exists.
                    auto itBindings = std::find_if(
                        descriptorSetLayoutBindings.begin(),
                        descriptorSetLayoutBindings.end(),
                        [binding](VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding)
                        {
                            return descriptorSetLayoutBinding.binding == binding;
                        });
                    // If doesn't already exist, create it.
                    if (itBindings == descriptorSetLayoutBindings.end())
                    {
                        specializationFunc(type, binding);

                        auto& descriptorSetLayoutBinding = descriptorSetLayoutBindings.emplace_back();
                        descriptorSetLayoutBinding.binding = binding;
                        descriptorSetLayoutBinding.descriptorType = descriptorType;
                        descriptorSetLayoutBinding.descriptorCount = count;

                        itBindings = descriptorSetLayoutBindings.end() - 1;
                    }
                    // Add the current stage to the binding.
                    itBindings->stageFlags |= stage;

                    // Get the descriptor pool size if it exists.
                    auto itSizes = std::find_if(
                        descriptorPoolSizes.begin(),
                        descriptorPoolSizes.end(),
                        [descriptorType](VkDescriptorPoolSize& size)
                        {
                            return size.type == descriptorType;
                        });
                    // If doesn't already exist, create it.
                    if (itSizes == descriptorPoolSizes.end())
                    {
                        auto& descriptorPoolSize = descriptorPoolSizes.emplace_back();
                        descriptorPoolSize.type = descriptorType;

                        itSizes = descriptorPoolSizes.end() - 1;
                    }
                    // Add the count to the descriptor pool size.
                    itSizes->descriptorCount += count;
                }
            };

            processResources(resources->uniform_buffers, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            [&](spirv_cross::SPIRType const& type, std::uint32_t binding)
            {
                UniformBufferInfo info;
                info.RenderContext = &m_Context;
                info.Size = reflection->get_declared_struct_size(type);
                for (auto& frame : m_FrameData)
                    frame.UniformBuffers.emplace_back(binding, std::make_unique<UniformBuffer>(info));
            });
            processResources(resources->storage_buffers, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            [&](spirv_cross::SPIRType const& type, std::uint32_t binding)
            {

            });
            processResources(resources->sampled_images, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            [&](spirv_cross::SPIRType const& type, std::uint32_t binding)
            {

            });
            // TODO: there's quite a few more resource types to handle.
        };

        // Since stage 0's reflection was already created, manually
        // process the rest of it before processing the other stages.
        processStage(std::get<1>(stages.front()));

        for (auto& stage : stages.subspan(1))
        {
            std::vector<std::uint8_t> const& code = std::get<0>(stage);

            // Recreate the reflection resources with the next stage, but using the existing memory.
            resources->~ShaderResources();
            reflection->~CompilerReflection();
            new (reflection.get()) spirv_cross::CompilerReflection((std::uint32_t const*)code.data(), code.size() / sizeof(std::uint32_t));
            new (resources.get()) spirv_cross::ShaderResources(reflection->get_shader_resources());

            processStage(std::get<1>(stage));
        }
    }

    void Shader::CreateDescriptorSetLayout(std::span<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings)
    {
        VkDevice device = m_Context.GetDevice();

        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = static_cast<std::uint32_t>(descriptorSetLayoutBindings.size());
        info.pBindings = descriptorSetLayoutBindings.data();

        VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &m_DescriptorSetLayout);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set layout.");
    }

    void Shader::CreateDescriptorPool(std::span<VkDescriptorPoolSize> descriptorPoolSizes)
    {
        VkDevice device = m_Context.GetDevice();
        std::uint32_t swapchainImageCount = m_Context.GetSwapchainImageCount();

        VkDescriptorPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.maxSets = static_cast<std::uint32_t>(descriptorPoolSizes.size() * swapchainImageCount);
        info.poolSizeCount = static_cast<std::uint32_t>(descriptorPoolSizes.size());
        info.pPoolSizes = descriptorPoolSizes.data();

        VkResult result = vkCreateDescriptorPool(device, &info, nullptr, &m_DescriptorPool);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool.");
    }

    void Shader::CreateDescriptorSets()
    {
        VkDevice device = m_Context.GetDevice();
        std::uint32_t swapchainImageCount = m_Context.GetSwapchainImageCount();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts(swapchainImageCount, m_DescriptorSetLayout);
        std::vector<VkDescriptorSet> descriptorSets(swapchainImageCount);

        VkDescriptorSetAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.descriptorPool = m_DescriptorPool;
        info.descriptorSetCount = static_cast<std::uint32_t>(descriptorSetLayouts.size());
        info.pSetLayouts = descriptorSetLayouts.data();

        VkResult result = vkAllocateDescriptorSets(device, &info, descriptorSets.data());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate descriptor sets.");

        // Copy the descriptor set handles.
        for (std::uint32_t i = 0; i < swapchainImageCount; i++)
            m_FrameData[i].DescriptorSet = descriptorSets[i];
    }
}
