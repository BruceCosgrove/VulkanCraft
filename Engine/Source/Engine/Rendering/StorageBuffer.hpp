#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Rendering/BufferUtils.hpp"
#include <span>

namespace eng
{
    class RenderContext;

    struct StorageBufferInfo
    {
        RenderContext* RenderContext = nullptr;
        u64 Size = 0; // In bytes
    };

    // NOTE: this assumes it will be updated every frame, and thus does not bother with a staging buffer.
    // NOTE: allocates enough memory for each frame in flight in the same buffer object.
    class StorageBuffer
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(StorageBuffer);

        StorageBuffer(StorageBufferInfo const& info);
        ~StorageBuffer();

        _ENG_BUFFER_SET_DATA(SetData);

        VkDeviceSize GetOffset() const;
        VkDeviceSize GetSize() const;
        VkBuffer GetBuffer() const;
    private:
        RenderContext& m_Context; // non-owning
        VkDeviceSize m_Size = 0;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
        std::span<u8> m_MappedMemory;
    };
}
