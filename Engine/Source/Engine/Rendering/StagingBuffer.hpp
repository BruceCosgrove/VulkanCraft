#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include <vulkan/vulkan.h>

namespace eng
{
    class RenderContext;

    struct StagingBufferInfo
    {
        RenderContext* RenderContext = nullptr;
        VkDeviceSize Size = 0;
        VkBufferUsageFlags Usage = 0;
    };

    // A temporary buffer whose destruction is deferred until after N frames in flight have completed.
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
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
        void* m_MappedMemory;
    };
}
