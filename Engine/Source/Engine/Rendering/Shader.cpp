#include "Shader.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/IO/FileIO.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include <shaderc/shaderc.hpp>
#include <array>
#include <ranges>
#include <vector>

namespace fs = std::filesystem;

namespace eng
{
    Shader::Shader(ShaderInfo const& info)
        : m_Context(*info.RenderContext)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        VkDevice device = m_Context.GetDevice();

        auto pipelineShaderStageInfos =
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
                    fs::path(info.Filepath).replace_extension(std::get<0>(stage)),
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
                shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(contents, kind, info.Filepath.string().c_str(), options);
                ENG_ASSERT(result.GetCompilationStatus() == shaderc_compilation_status_success, "Failed to compile shader: {}", result.GetErrorMessage());

                // Return the spir-v bytecode and the vulkan stage flags.
                return std::make_tuple(
                    std::vector((std::uint8_t*)result.begin(), (std::uint8_t*)result.end()),
                    std::get<2>(stage)
                );
            }) |

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
            std::views::transform([](auto&& stage)
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

        auto dynamicStates = std::to_array({
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        });

        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
        dynamicStateInfo.pDynamicStates = dynamicStates.data();

        VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
        vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        // TODO: these are per vertex buffer bound to the pipeline.
        vertexInputStateInfo.vertexBindingDescriptionCount = 0;
        vertexInputStateInfo.pVertexBindingDescriptions = nullptr;
        // TODO: these are per input attribute, e.g. "layout(location=0) in vec3 i_Position;".
        // their bindings reference into the vertexInputStateInfo.pVertexBindingDescriptions array.
        vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateInfo.pVertexAttributeDescriptions = nullptr;

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
        // TODO: descriptor pool
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;

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
    }

    void Shader::Bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

        // TODO: vkCmdBindDescriptorSets
    }
}
