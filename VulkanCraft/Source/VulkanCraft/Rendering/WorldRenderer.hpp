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

    class WorldRenderer
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(WorldRenderer);
    public:
        WorldRenderer(RenderContext& context, VkRenderPass renderPass, u16 maxChunkCount);
        ~WorldRenderer();

        void Render(VkCommandBuffer commandBuffer, World const& world, mat4 const& viewProjection);
        void ReloadShaders();
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

        struct ChunkRegion
        {
            VkDeviceSize VertexOffset;
            VkDeviceSize VertexSize;
        };

        struct ChunkSubmeshRegion
        {
            u32 RegionIndex; // Index into m_ChunkRegions that this submesh corresponds to.
            u32 FirstInstance;
            u16 InstanceCount; // NOTE: max is 4096 = 2^12 < 2^16-1, so using u16 is fine.
            MeshType Type;
        };
    private:
        void AddOrReplaceChunkMesh(Chunk* chunk);
        void RemoveChunkMesh(Chunk* chunk);
        void RemoveChunkMesh(std::unordered_multimap<Chunk*, ChunkSubmeshRegion>::iterator& it);

        std::shared_ptr<Shader> LoadShader();
    private:
        RenderContext& m_Context; // non-owning
        VkRenderPass m_RenderPass; // non-owning
        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
        std::vector<VkBuffer> m_UniformBuffers;
        std::vector<VkBuffer> m_StorageBuffers;
        std::vector<VkBuffer> m_IndirectBuffers;
        std::vector<VkDeviceSize> m_UniformOffsets;
        std::vector<VkDeviceSize> m_StorageOffsets;
        std::vector<VkDeviceSize> m_IndirectOffsets;
        VkDeviceMemory m_DeviceLocalMemory = VK_NULL_HANDLE;
        VkDeviceMemory m_HostVisibleMemory = VK_NULL_HANDLE;
        std::span<u8> m_MappedMemory;

        DynamicResource<std::shared_ptr<Shader>> m_Shader;
        std::unique_ptr<TextureAtlas> m_BlockTextureAtlas;

        // List of used regions in the associated buffers.
        std::vector<ChunkRegion> m_ChunkRegions;
        // Maps chunks to their submesh instance indices.
        std::unordered_multimap<Chunk*, ChunkSubmeshRegion> m_ChunkSubmeshRegions;

        // Debug visualization

        std::atomic<u8> m_Wireframe = 0;
    };
}
