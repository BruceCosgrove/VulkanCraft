#pragma once

#include "Engine/Rendering/_Buffer.hpp"
#include <span>

namespace eng
{
    class RenderContext;

    struct UniformBufferInfo
    {
        RenderContext* RenderContext = nullptr;
        std::uint64_t Size = 0; // In bytes
    };

    // NOTE: this assumes it will be updated every frame, and thus does not bother with a staging buffer.
    // NOTE: allocates enough memory for each frame in flight in the same buffer object.
    class UniformBuffer : private detail::Buffer
    {
    public:
        UniformBuffer(UniformBufferInfo const& info);
        ~UniformBuffer();

        void SetData(std::span<std::uint8_t const> data);
        _ENG_BUFFER_SET_ARBITRARY_DATA(SetData)

        VkDeviceSize GetOffset() const;
        VkDeviceSize GetSize() const;
        VkBuffer GetBuffer() const;
    private:
        VkDeviceSize m_Size = 0;
        VkDeviceSize m_AlignedSize = 0;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
        std::span<std::uint8_t> m_MappedMemory;
    };
}
