#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Rendering/BufferUtils.hpp"
#include <span>

namespace eng
{
    struct VertexBufferInfo
    {
        RenderContext* RenderContext = nullptr;
        u64 Size = 0; // In bytes
    };

    // NOTE: this assumes it will be updated every frame, and thus does not bother with a staging buffer.
    class VertexBuffer
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(VertexBuffer);
    public:
        VertexBuffer(VertexBufferInfo const& info);
        ~VertexBuffer();

        _ENG_BUFFER_SET_DATA(SetData);

        void Bind(VkCommandBuffer commandBuffer, u32 binding = 0);
    private:
        RenderContext& m_Context; // non-owning
        VkDeviceSize m_Size = 0;
        VkBuffer m_Buffer = nullptr;
        VkDeviceMemory m_DeviceMemory = nullptr;
        void* m_MappedMemory = nullptr;
    };
}
