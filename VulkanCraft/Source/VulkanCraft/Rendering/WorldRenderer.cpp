#include "WorldRenderer.hpp"
#include "VulkanCraft/World/World.hpp"

namespace vc
{
    WorldRenderer::WorldRenderer(RenderContext& context, VkRenderPass renderPass, u16 maxChunkCount)
        : m_Context(context)
        , m_RenderPass(renderPass)
    {
        ReloadShaders();

        // TODO: where to actually put this loading?
        // TODO: dynamic loading via block model files
        {
#define VC_TEXTURE(x) R"(D:\Dorkspace\Programming\Archive\VanillaDefault-Resource-Pack-16x-1.21\assets\minecraft\textures\)" x
            std::vector<LocalTexture> textures;
            textures.reserve(5);
            textures.emplace_back(VC_TEXTURE("block/bedrock.png"));
            textures.emplace_back(VC_TEXTURE("block/stone.png"));
            textures.emplace_back(VC_TEXTURE("block/dirt.png"));
            textures.emplace_back(VC_TEXTURE("block/grass_block_side.png"));
            textures.emplace_back(VC_TEXTURE("block/grass_block_top.png"));
            m_BlockTextureAtlas = std::make_unique<TextureAtlas>(m_Context, 16, textures);
        }

        VkDevice device = m_Context.GetDevice();
        u32 swapchainImageCount = m_Context.GetSwapchainImageCount();
        
        m_UniformBuffers.resize(swapchainImageCount);
        m_StorageBuffers.resize(swapchainImageCount);
        m_IndirectBuffers.resize(swapchainImageCount);
        m_UniformOffsets.resize(swapchainImageCount);
        m_StorageOffsets.resize(swapchainImageCount);
        m_IndirectOffsets.resize(swapchainImageCount);

        // chunks * blocks/chunk * faces/block * bytes/face
        VkDeviceSize vertexBufferSize = maxChunkCount * (Chunk::Size3 * 6 * sizeof(uvec2));
        // Use the same struct layout as the GPU will have and simply use sizeof.
        VkDeviceSize uniformBufferSize = sizeof(LocalUniformBuffer);
        // chunks * faces/chunk * bytes/face
        VkDeviceSize storageBufferSize = maxChunkCount * (6 * sizeof(uvec2));
        // chunks * faces/chunk * bytes/face
        VkDeviceSize indirectBufferSize = maxChunkCount * (6 * sizeof(VkDrawIndirectCommand));

        // Create vertex buffer and get its memory requirements.
        VkMemoryRequirements vertexMemoryRequirements;
        {
            VkBufferCreateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = vertexBufferSize,
                .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            };
            VkResult result = vkCreateBuffer(device, &info, nullptr, &m_VertexBuffer);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create vertex buffer.");
            vkGetBufferMemoryRequirements(device, m_VertexBuffer, &vertexMemoryRequirements);
        }

        // Create uniform buffers and get their memory requirements.
        VkMemoryRequirements uniformMemoryRequirements;
        {
            VkBufferCreateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = uniformBufferSize,
                .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            };
            for (auto& uniformBuffer : m_UniformBuffers)
            {
                VkResult result = vkCreateBuffer(device, &info, nullptr, &uniformBuffer);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to create uniform buffer.");
            }
            // NOTE: Assumes all uniform buffer memory requirements are the same.
            vkGetBufferMemoryRequirements(device, m_UniformBuffers.front(), &uniformMemoryRequirements);
        }

        // Create storage buffers and get their memory requirements.
        VkMemoryRequirements storageMemoryRequirements;
        {
            VkBufferCreateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = storageBufferSize,
                .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            };
            for (auto& storageBuffer : m_StorageBuffers)
            {
                VkResult result = vkCreateBuffer(device, &info, nullptr, &storageBuffer);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to create storage buffer.");
            }
            // NOTE: Assumes all uniform buffer memory requirements are the same.
            vkGetBufferMemoryRequirements(device, m_StorageBuffers.front(), &storageMemoryRequirements);
        }

        // Create indirect buffers and get their memory requirements.
        VkMemoryRequirements indirectMemoryRequirements;
        {
            VkBufferCreateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = indirectBufferSize,
                .usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            };
            for (auto& indirectBuffer : m_IndirectBuffers)
            {
                VkResult result = vkCreateBuffer(device, &info, nullptr, &indirectBuffer);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to create indirect buffer.");
            }
            // NOTE: Assumes all uniform buffer memory requirements are the same.
            vkGetBufferMemoryRequirements(device, m_IndirectBuffers.front(), &indirectMemoryRequirements);
        }

        // Calculate buffer offsets and allocation sizes.
        VkDeviceSize vertexOffset;
        VkDeviceSize deviceLocalAllocationSize, hostVisibleAllocationSize;
        {
            VkDeviceSize offset = 0;
            vertexOffset = BufferUtils::Align(offset, vertexMemoryRequirements.alignment);
            offset += vertexMemoryRequirements.size;

            VkDeviceSize maxAlignment = vertexMemoryRequirements.alignment;
            deviceLocalAllocationSize = BufferUtils::Align(offset, maxAlignment);

            offset = 0;
            for (auto& uniformOffset : m_UniformOffsets)
            {
                uniformOffset = BufferUtils::Align(offset, uniformMemoryRequirements.alignment);
                offset += uniformMemoryRequirements.size;
            }
            for (auto& storageOffset : m_StorageOffsets)
            {
                storageOffset = BufferUtils::Align(offset, storageMemoryRequirements.alignment);
                offset += storageMemoryRequirements.size;
            }
            for (auto& indirectOffset : m_IndirectOffsets)
            {
                indirectOffset = BufferUtils::Align(offset, indirectMemoryRequirements.alignment);
                offset += indirectMemoryRequirements.size;
            }

            maxAlignment = std::max({
                uniformMemoryRequirements.alignment,
                storageMemoryRequirements.alignment,
                indirectMemoryRequirements.alignment,
            });
            hostVisibleAllocationSize = BufferUtils::Align(offset, maxAlignment);
        }

        // Allocate once for all device local buffers.
        {
            u32 memoryTypeBits = vertexMemoryRequirements.memoryTypeBits;
            VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            VkMemoryAllocateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = deviceLocalAllocationSize,
                .memoryTypeIndex = BufferUtils::SelectMemoryType(memoryTypeBits, properties),
            };
            VkResult result = vkAllocateMemory(device, &info, nullptr, &m_DeviceLocalMemory);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate device local memory.");
        }

        // Allocate once for all host visible buffers.
        {
            ENG_ASSERT(
                uniformMemoryRequirements.memoryTypeBits == storageMemoryRequirements.memoryTypeBits and
                storageMemoryRequirements.memoryTypeBits == indirectMemoryRequirements.memoryTypeBits
            );
            u32 memoryTypeBits = uniformMemoryRequirements.memoryTypeBits;
            VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            VkMemoryAllocateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = hostVisibleAllocationSize,
                .memoryTypeIndex = BufferUtils::SelectMemoryType(memoryTypeBits, properties),
            };
            VkResult result = vkAllocateMemory(device, &info, nullptr, &m_HostVisibleMemory);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate host visible memory.");
        }

        // Bind the memory to each of the buffers at the appropriate offsets.
        {
            VkResult result = vkBindBufferMemory(device, m_VertexBuffer, m_DeviceLocalMemory, vertexOffset);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to bind vertex buffer memory.");
            for (u32 i = 0; i < swapchainImageCount; i++)
            {
                result = vkBindBufferMemory(device, m_UniformBuffers[i], m_HostVisibleMemory, m_UniformOffsets[i]);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to bind uniform buffer {} memory.", i);
                result = vkBindBufferMemory(device, m_StorageBuffers[i], m_HostVisibleMemory, m_StorageOffsets[i]);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to bind storage buffer {} memory.", i);
                result = vkBindBufferMemory(device, m_IndirectBuffers[i], m_HostVisibleMemory, m_IndirectOffsets[i]);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to bind indirect buffer {} memory.", i);
            }
        }

        void* mappedMemory;
        BufferUtils::MapMemory(m_Context, m_HostVisibleMemory, 0, hostVisibleAllocationSize, mappedMemory);
        m_MappedMemory = std::span((u8*)mappedMemory, hostVisibleAllocationSize);
    }

    WorldRenderer::~WorldRenderer()
    {
        VkDevice device = m_Context.GetDevice();
        u32 swapchainImageCount = m_Context.GetSwapchainImageCount();

        BufferUtils::UnmapMemory(m_Context, m_HostVisibleMemory);
        vkDestroyBuffer(device, m_VertexBuffer, nullptr);
        for (auto& uniformBuffer : m_UniformBuffers)
            vkDestroyBuffer(device, uniformBuffer, nullptr);
        for (auto& storageBuffer : m_StorageBuffers)
            vkDestroyBuffer(device, storageBuffer, nullptr);
        for (auto& indirectBuffer : m_IndirectBuffers)
            vkDestroyBuffer(device, indirectBuffer, nullptr);
        vkFreeMemory(device, m_DeviceLocalMemory, nullptr);
        vkFreeMemory(device, m_HostVisibleMemory, nullptr);
    }

    auto WorldRenderer::Render(VkCommandBuffer commandBuffer, World const& world, mat4 const& viewProjection) -> Statistics
    {
        // TODO: use compute shader to decide which chunks get rendered rather than cpu-side decisions.

        u32 swapchainImageIndex = m_Context.GetSwapchainImageIndex();

        auto& uniformBuffer = m_UniformBuffers[swapchainImageIndex];
        auto& storageBuffer = m_StorageBuffers[swapchainImageIndex];
        auto& indirectBuffer = m_IndirectBuffers[swapchainImageIndex];
        auto& uniformOffset = m_UniformOffsets[swapchainImageIndex];
        auto& storageOffset = m_StorageOffsets[swapchainImageIndex];
        auto& indirectOffset = m_IndirectOffsets[swapchainImageIndex];

        // Remove regions for all chunks that have been removed from the world.
        for (auto it = m_ChunkSubmeshRegions.begin(); it != m_ChunkSubmeshRegions.end(); )
        {
            if (not world.m_Chunks.contains(it->first->GetPosition()))
                RemoveChunkMesh(it);
            else
                std::advance(it, m_ChunkSubmeshRegions.count(it->first));
        }

        // Add regions for all chunks that have not yet been added from the world.
        {
            VkCommandBuffer commandBuffer = m_Context.BeginOneTimeCommandBuffer();
            for (auto& [position, chunk] : world.m_Chunks)
            {
                if (not m_ChunkSubmeshRegions.contains(chunk.get()) and not chunk->IsGenerating())
                {
                    ChunkGenerationStage stage = chunk->GetGenerationStage();
                    switch (+stage)
                    {
                        case ChunkGenerationStage::Mesh - 1:
                            chunk->GenerateMesh();
                            break;
                        case ChunkGenerationStage::Mesh:
                            AddOrReplaceChunkMesh(commandBuffer, chunk.get());
                            break;
                    }
                }
            }
            m_Context.EndOneTimeCommandBuffer(commandBuffer);
        }

        // TODO: cull chunks
        auto& renderingRegions = m_ChunkSubmeshRegions;
        u32 drawCount = u32(renderingRegions.size());

        auto shader = m_Shader.Get();

        // No point in doing anything if nothing will be rendered.

        if (drawCount == 0 or not shader.Current)
            return {};

        // Set uniform buffer data.
        VkDeviceSize uniformSize = sizeof(LocalUniformBuffer);
        {
            LocalUniformBuffer localUniformBuffer
            {
                .ViewProjection = viewProjection,
                .BlockTextureAtlas
                {
                    .TextureCount = m_BlockTextureAtlas->GetTextureCount(),
                    .TextureScale = m_BlockTextureAtlas->GetTextureScale(),
                    .TexturesPerLayer = m_BlockTextureAtlas->GetTexturesPerLayer(),
                },
            };

            std::memcpy(m_MappedMemory.data() + uniformOffset, &localUniformBuffer, uniformSize);
        }

        // Set storage buffer data.
        VkDeviceSize storageSize = drawCount * sizeof(uvec2);
        {
            std::vector<uvec2> storageData;
            storageData.reserve(drawCount);

            //                  packedChunkData.y                packedChunkData.x
            // 64  60  56  52  48  44  40  36  32   28  24  20  16  12   8   4   0
            //   -------------fffzzzzzzzzzzzzzzzz yyyyyyyyyyyyyyyyxxxxxxxxxxxxxxxx
            // x = chunk x, y = chunk y, z = chunk z, f = face
            auto packStorageData = [](Chunk* chunk, MeshType type) -> uvec2
            {
                ChunkPos position = chunk->GetPosition() & 0xFFFF;
                return
                {
                    position.y << 16 | position.x,
                    /* extra 13 bits */ (+type - MeshType::_Begin) << 16 | position.z,
                };
            };

            for (auto& [chunk, region] : renderingRegions)
                storageData.emplace_back(packStorageData(chunk, region.Type));

            std::memcpy(m_MappedMemory.data() + storageOffset, storageData.data(), storageSize);
        }

        // Set indirect buffer data.
        VkDeviceSize indirectSize = drawCount * sizeof(VkDrawIndirectCommand);
        {
            std::vector<VkDrawIndirectCommand> indirectData;
            indirectData.reserve(drawCount);

            for (auto& [chunk, region] : renderingRegions)
                indirectData.emplace_back(6, region.InstanceCount, 0, region.FirstInstance);

            std::memcpy(m_MappedMemory.data() + indirectOffset, indirectData.data(), indirectSize);
        }

        // Update shader descriptors and bind shader.
        {
            if (shader.Old)
                m_Context.DeferFree([oldShader = std::move(shader.Old)] {});

            auto uniformBuffers = std::to_array<ShaderUniformBufferBinding>
            ({
                {0, uniformBuffer, 0, uniformSize},
            });
            auto storageBuffers = std::to_array<ShaderStorageBufferBinding>
            ({
                {1, storageBuffer, 0, storageSize},
            });
            auto samplers = std::to_array<ShaderSamplerBinding>
            ({
                {2, m_BlockTextureAtlas->GetSampler(), m_BlockTextureAtlas->GetTexture()->GetImageView()},
            });

            shader.Current->UpdateDescriptorSet({uniformBuffers, storageBuffers, samplers});
            shader.Current->Bind(commandBuffer);
        }

        // Debug visualization.
        {
            ENG_GET_FUNC_VK_EXT(vkCmdSetPolygonModeEXT);
            VkPolygonMode polygonMode = m_Wireframe.load(std::memory_order_relaxed) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
            vkCmdSetPolygonModeEXT(commandBuffer, polygonMode);
        }

        // Set viewport and scissor.
        {
            VkExtent2D extent = m_Context.GetSwapchainExtent();

            // Vulkan's +y direction is down, this fixes that.
            // https://stackoverflow.com/questions/45570326/flipping-the-viewport-in-vulkan
            VkViewport viewport
            {
                .x = 0.0f,
                .y = f32(extent.height),
                .width = f32(extent.width),
                .height = -f32(extent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor
            {
                .offset{0, 0},
                .extent = extent,
            };
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        }

        // Bind the vertex buffer.
        {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer, &offset);
        }

        // Draw everything.
        vkCmdDrawIndirect(commandBuffer, indirectBuffer, 0, drawCount, sizeof(VkDrawIndirectCommand));

        return Statistics
        {
            .IndirectDrawCallCount = drawCount,
            .InstanceCount = m_TotalInstanceCount,
            .ChunkCount = u32(m_ChunkRegions.size()),
            .UsedVertexBufferSize = m_UsedVertexBufferSize,
            .UsedUniformBufferSize = uniformSize,
            .UsedStorageBufferSize = storageSize,
            .UsedIndirectBufferSize = indirectSize,
        };
    }

    void WorldRenderer::ReloadShaders()
    {
        if (not m_Shader.Loading())
        {
            Application::Get().ExecuteAsync([this]
            {
                m_Shader.Load([this] { return LoadShader(); });
            });
        }
    }

    void WorldRenderer::ToggleWireframe()
    {
        m_Wireframe.fetch_xor(1, std::memory_order_relaxed);
    }

    void WorldRenderer::AddOrReplaceChunkMesh(VkCommandBuffer commandBuffer, Chunk* chunk)
    {
        ChunkMeshData meshData = *chunk->ConsumeGenerationStageOutput<ChunkMeshData>();

        struct Submesh
        {
            VkDeviceSize VertexOffset; // offset in bytes from the start of the staging buffer
            VkDeviceSize VertexSize;   // size in bytes in the staging buffer
            u32 FirstInstance;
            u16 InstanceCount;
        };
        std::array<Submesh, +MeshType::_Count> submeshes{};

        // Compute the vertex size and instance count of each submesh and the entire mesh.
        VkDeviceSize meshVertexSize = 0;
        u32 meshInstanceCount = 0;
        auto calculateSubmeshBounds = [&](std::vector<uvec2> const& data, Submesh& submesh)
        {
            submesh.VertexOffset = meshVertexSize;
            submesh.VertexSize = data.size() * sizeof(uvec2); // size of vertex

            submesh.FirstInstance = meshInstanceCount;
            submesh.InstanceCount = u16(data.size());

            meshVertexSize += submesh.VertexSize;
            meshInstanceCount += submesh.InstanceCount;
        };

        for (u8 i = 0; i < MeshType::_Count; i++)
            calculateSubmeshBounds((&meshData.Left)[i], submeshes[i]);

        // Copy the data to the staging buffer.
        VkBuffer stagingBuffer;
        {
            // Create the staging buffer.
            StagingBuffer buffer({&m_Context, meshVertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT});

            u8* stagingMappedMemory = (u8*)buffer.GetMappedMemory();
            for (u8 i = 0; i < MeshType::_Count; i++)
                std::memcpy(stagingMappedMemory + submeshes[i].VertexOffset, (&meshData.Left)[i].data(), submeshes[i].VertexSize);

            stagingBuffer = buffer.GetBuffer();
        }

        // If replacing this chunk mesh, first remove the old data.
        if (auto it = m_ChunkSubmeshRegions.find(chunk); it != m_ChunkSubmeshRegions.end())
            RemoveChunkMesh(it);

        // Find the smallest region large enough to fit the mesh.
        VkDeviceSize vertexOffset = 0;
        for (auto& region : m_ChunkRegions)
        {
            if (region.VertexOffset - vertexOffset >= meshVertexSize)
                break;
            vertexOffset += region.VertexSize;
        }

        // Add the submesh regions.
        for (u8 i = 0; i < MeshType::_Count; i++)
        {
            if (submeshes[i].InstanceCount)
            {
                m_ChunkSubmeshRegions.insert({chunk,
                {
                    u32(m_ChunkRegions.size()),
                    submeshes[i].FirstInstance + u32(vertexOffset / sizeof(uvec2)),
                    submeshes[i].InstanceCount,
                    MeshType(MeshType::_Begin + i),
                }});
            }
        }

        // Add the region.
        m_ChunkRegions.emplace_back(vertexOffset, meshVertexSize);
        m_TotalInstanceCount += meshInstanceCount;
        m_UsedVertexBufferSize += meshVertexSize;

        // Copy the staging buffer to the vertex buffer.
        {
            VkBufferCopy region
            {
                .srcOffset = 0,
                .dstOffset = vertexOffset,
                .size = meshVertexSize,
            };
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, m_VertexBuffer, 1, &region);
        }
    }

    void WorldRenderer::RemoveChunkMesh(Chunk* chunk)
    {
        // Get an iterator to the first submesh this chunk has.
        auto it = m_ChunkSubmeshRegions.find(chunk);
        RemoveChunkMesh(it);
    }

    void WorldRenderer::RemoveChunkMesh(std::unordered_multimap<Chunk*, ChunkSubmeshRegion>::iterator& it)
    {
        ENG_ASSERT(it != m_ChunkSubmeshRegions.end());
        auto endEraseIt = std::next(it, m_ChunkSubmeshRegions.count(it->first));
        // Get the index into the "allocations" so it can be "deallocated".
        u32 index = it->second.RegionIndex;

        for (decltype(auto) it2 = it; it2 != endEraseIt; ++it2)
            m_TotalInstanceCount -= it->second.InstanceCount;
        m_UsedVertexBufferSize -= m_ChunkRegions[index].VertexSize;

        // Remove all submeshes this chunk has.
        // NOTE: This can be removed immediately because it has already been copied to the indirect buffer.
        it = m_ChunkSubmeshRegions.erase(it, endEraseIt);
        // Wait until all previous frames have stopped using the old chunk data to remove the "allocation".
        m_Context.DeferFree([this, index]
        {
            // Let other other chunks use the vertex and storage buffer in this region again.
            m_ChunkRegions.erase(m_ChunkRegions.begin() + index);
        });
    }

    std::shared_ptr<Shader> WorldRenderer::LoadShader()
    {
        auto bindings = std::to_array<ShaderVertexBufferBinding>
        ({
            {0, VK_VERTEX_INPUT_RATE_INSTANCE, {0}},
        });

        ShaderInfo info;
        info.RenderContext = &m_Context;
        info.Filepath = "Assets/Shaders/Chunk";
        info.VertexBufferBindings = bindings;
        info.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        info.RenderPass = m_RenderPass;

        ENG_LOG_INFO("Loading shader \"{}\"...", info.Filepath.string());
        return std::make_shared<Shader>(info);
    }
}
