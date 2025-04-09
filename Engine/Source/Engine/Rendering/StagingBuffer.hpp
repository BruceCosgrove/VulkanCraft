#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include <vulkan/vulkan.h>

namespace eng
{
    class RenderContext;

    struct StagingBufferInfo
    {
        RenderContext* RenderContext = nullptr;
        u64 Size = 0;
        VkBufferUsageFlags Usage = 0;
    };

    // A temporary buffer to transfer data to long-lived buffers.
    class StagingBuffer
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(StagingBuffer);
    public:
        StagingBuffer(StagingBufferInfo const& info);
        ~StagingBuffer();

        void* GetMappedMemory();
        VkBuffer GetBuffer() const;
    private:
        RenderContext& m_Context; // non-owning
        VkBuffer m_Buffer = nullptr;
        VkDeviceMemory m_DeviceMemory = nullptr;
        void* m_MappedMemory;
    };
}
