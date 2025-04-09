#pragma once

#include "Engine/Rendering/BufferUtils.hpp"

namespace eng
{
    class RenderContext;

    struct UniformBufferInfo
    {
        RenderContext* RenderContext = nullptr;
        u64 Size = 0; // In bytes
    };

    // NOTE: this assumes it will be updated every frame, and thus does not bother with a staging buffer.
    // NOTE: allocates enough memory for each frame in flight in the same buffer object.
    class UniformBuffer
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(UniformBuffer);
    public:
        UniformBuffer(UniformBufferInfo const& info);
        ~UniformBuffer();

        _ENG_BUFFER_SET_DATA(SetData);

        VkDeviceSize GetOffset() const;
        VkDeviceSize GetSize() const;
        VkBuffer GetBuffer() const;
    private:
        RenderContext& m_Context; // non-owning
        VkDeviceSize m_Size = 0;
        VkBuffer m_Buffer = nullptr;
        VkDeviceMemory m_DeviceMemory = nullptr;
        std::span<u8> m_MappedMemory;
    };
}
