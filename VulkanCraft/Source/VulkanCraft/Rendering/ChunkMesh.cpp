#include "ChunkMesh.hpp"

namespace vc
{
    ChunkMesh::ChunkMesh(ChunkMeshInfo const& info)
        : m_Context(*info.RenderContext)
    {
        // Compute the offset and size of each submesh and the size of the entire buffer.
        VkDeviceSize size = 0;

        auto calculateSubmeshBounds = [&size]<typename T>(std::vector<T> const& data, Submesh& submesh)
        {
            // The size, in bytes, of the submesh data.
            VkDeviceSize submeshSize = data.size() * sizeof(T);
            // Account for alignment of T
            size = (size + alignof(T) - 1) & ~(alignof(T) - 1);

            submesh.Offset = size;
            submesh.Size = submeshSize;
            size += submeshSize;
        };

        for (u8 i = 0; i < MeshType::_Count; i++)
            calculateSubmeshBounds((&info.Left)[i], m_Submeshes[i]);

        // Create the staging buffer.
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingDeviceMemory = VK_NULL_HANDLE;
        BufferUtils::CreateBuffer(
            m_Context,
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingDeviceMemory
        );

        // Copy the data to the staging buffer.
        u8* stagingMappedMemory = nullptr;
        BufferUtils::MapMemory(m_Context, stagingDeviceMemory, 0, size, (void*&)stagingMappedMemory);
        for (u8 i = 0; i < MeshType::_Count; i++)
            std::memcpy(stagingMappedMemory + m_Submeshes[i].Offset, (&info.Left)[i].data(), m_Submeshes[i].Size);
        BufferUtils::UnmapMemory(m_Context, stagingDeviceMemory);

        // Create the actual buffer.
        BufferUtils::CreateBuffer(
            m_Context,
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_Buffer,
            m_DeviceMemory
        );

        // Copy the staging buffer to the actual buffer.
        VkCommandBuffer commandBuffer = m_Context.BeginOneTimeCommandBuffer();
        VkBufferCopy region{};
        region.size = size;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, m_Buffer, 1, &region);
        m_Context.EndOneTimeCommandBuffer(commandBuffer);

        // Destroy the staging buffer.
        VkDevice device = m_Context.GetDevice();
        vkFreeMemory(device, stagingDeviceMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
    }

    ChunkMesh::~ChunkMesh()
    {
        VkDevice device = m_Context.GetDevice();
        vkFreeMemory(device, m_DeviceMemory, nullptr);
        vkDestroyBuffer(device, m_Buffer, nullptr);
    }

    void ChunkMesh::Draw(VkCommandBuffer commandBuffer)
    {
        for (u8 i = 0; i < MeshType::_Count; i++)
        {
            if (m_Submeshes[i].Size > 0)
            {
                VkDeviceSize offset = m_Submeshes[i].Offset;
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_Buffer, &offset);
                vkCmdDraw(commandBuffer, 6, 1, 0, 0);
            }
        }
    }
}
