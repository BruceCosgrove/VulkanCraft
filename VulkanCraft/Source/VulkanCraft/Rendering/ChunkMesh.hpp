#pragma once

#include "VulkanCraft/Rendering/MeshType.hpp"
#include <Engine.hpp>
#include <array>
#include <vector>

using namespace eng;

namespace vc
{
    // Notes:
    // Each chunk will have its own mesh.
    // Mesh generation will have different vectors for each submesh.
    // Then to upload them, they'll be flattened into a single VkBuffer with offsets.
    // 
    // Since the vast majority of chunk meshes won't change between frames, these
    // meshes will have a staging buffer. For frames in flight, simply wait for
    // the most recent frame using a mesh to finish until destroying the mesh.

    struct ChunkMeshInfo
    {
        RenderContext* RenderContext = nullptr;
        std::vector<uvec2> Left;
        std::vector<uvec2> Right;
        std::vector<uvec2> Bottom;
        std::vector<uvec2> Top;
        std::vector<uvec2> Back;
        std::vector<uvec2> Front;
    };

    class ChunkMesh
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(ChunkMesh);
    public:
        ChunkMesh(ChunkMeshInfo const& info);
        ~ChunkMesh();

        // TODO: this is gonna be replaced, so it's implementation doesn't really matter right now.
        void Draw(VkCommandBuffer commandBuffer);
    private:
        RenderContext& m_Context; // non-owning
        VkDeviceSize m_BufferSize = 0;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;

        struct Submesh
        {
            VkDeviceSize Offset = 0;
            VkDeviceSize Size = 0;
        };
        std::array<Submesh, +MeshType::_Count> m_Submeshes;
    };
}
