#pragma once

#include "VulkanCraft/Rendering/ChunkMeshData.hpp"
#include "VulkanCraft/Rendering/MeshType.hpp"
#include "VulkanCraft/Rendering/TextureAtlas.hpp"
#include <Engine.hpp>

using namespace eng;

namespace vc
{
    class Chunk;
    class World;
    class ChunkGenerator;

    class WorldRenderer
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(WorldRenderer);
    public:
        WorldRenderer(RenderContext& context, VkRenderPass renderPass, u16 maxChunkCount);
        ~WorldRenderer();

        struct Statistics
        {
            u32 IndirectDrawCallCount = 0;
            u32 InstanceCount = 0;
            u32 ChunkCount = 0;
            u64 UsedVertexBufferSize = 0;
            u64 UsedUniformBufferSize = 0;
            u64 UsedStorageBufferSize = 0;
            u64 UsedIndirectBufferSize = 0;
        };

        Statistics Render(
            VkCommandBuffer commandBuffer,
            mat4 const& viewProjection,
            World const& world,
            ChunkGenerator& chunkGenerator
        );

        // Can be called from any thread.
        void ReloadShaders();
        // Can be called from any thread.
        void ToggleWireframe();
    private:
        struct LocalUniformBuffer
        {
            alignas(16) mat4 ViewProjection;

            struct alignas(16)
            {
                alignas(8) uvec3 TextureCount;
                alignas(8) vec2 TextureScale;    // 1.0f / TextureCount
                alignas(4) u32 TexturesPerLayer; // TextureCount.x * TextureCount.y
            } BlockTextureAtlas;
        };

        struct ChunkSubmeshRegion
        {
            ChunkPos ChunkPos;
            u32 FirstInstance;
            u16 InstanceCount; // NOTE: max is 4096 = 2^12 < 2^16-1, so using u16 is fine.
            MeshType Type;
            bool Removing = false;
        };
    private:
        void AddOrReplaceChunkMesh(VkCommandBuffer commandBuffer, ChunkMeshData const& meshData);
        // Removes a chunk mesh if one at the given position exists.
        void RemoveChunkMesh(ChunkPos chunkPos);

        std::shared_ptr<Shader> LoadShaders();
    private:
        RenderContext& m_Context; // non-owning
        VkRenderPass m_RenderPass; // non-owning
        VkBuffer m_VertexBuffer = nullptr;
        std::vector<VkBuffer> m_UniformBuffers;
        std::vector<VkBuffer> m_StorageBuffers;
        std::vector<VkBuffer> m_IndirectBuffers;
        std::vector<VkDeviceSize> m_UniformOffsets;
        std::vector<VkDeviceSize> m_StorageOffsets;
        std::vector<VkDeviceSize> m_IndirectOffsets;
        VkDeviceMemory m_DeviceLocalMemory = nullptr;
        VkDeviceMemory m_HostVisibleMemory = nullptr;
        std::span<u8> m_MappedMemory;

        DynamicResource<std::shared_ptr<Shader>> m_Shader;
        std::unique_ptr<TextureAtlas> m_BlockTextureAtlas;

        // Maps of chunk positions to their regions.
        std::vector<ChunkSubmeshRegion> m_ChunkSubmeshRegions;

        // Statistics

        u64 m_UsedVertexBufferSize = 0;
        u32 m_TotalInstanceCount = 0;
        u32 m_TotalChunkCount = 0;

        // Debug visualization

        std::atomic<u8> m_Wireframe = 0;
    };
}
