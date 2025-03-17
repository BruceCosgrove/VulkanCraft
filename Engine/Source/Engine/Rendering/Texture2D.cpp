#include "Texture2D.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/BufferUtils.hpp"
#include "Engine/Rendering/ImageUtils.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    Texture2D::Texture2D(Texture2DInfo const& info)
        : Image(info.RenderContext)
    {
        // Create the staging buffer.
        VkDeviceSize stagingSize = info.LocalTexture->GetSize();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingDeviceMemory;

        BufferUtils::CreateBuffer(
            m_Context,
            stagingSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingDeviceMemory
        );

        // Copy the texture data to the staging buffer.
        void* mappedMemory;
        BufferUtils::MapMemory(m_Context, stagingDeviceMemory, 0, stagingSize, mappedMemory);
        std::memcpy(mappedMemory, info.LocalTexture->GetPixels2D().data_handle(), stagingSize);
        BufferUtils::UnmapMemory(m_Context, stagingDeviceMemory);

        // Create the image.
        VkExtent3D extent = {info.LocalTexture->GetWidth(), info.LocalTexture->GetHeight(), 1};

        ImageUtils::CreateImage(
            m_Context,
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_2D,
            info.Format,
            extent,
            1,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            m_Image,
            m_ImageView,
            m_DeviceMemory
        );

        VkCommandBuffer commandBuffer = m_Context.BeginOneTimeCommandBuffer();
        // Transition the image layout to be written to.
        ImageUtils::TransitionImageLayout(commandBuffer, m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
        // Copy the texture from the staging buffer to the image.
        ImageUtils::CopyBufferToImage(commandBuffer, stagingBuffer, m_Image, extent, 1);
        // Transition the image layout to be read from shaders.
        ImageUtils::TransitionImageLayout(commandBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
        m_Context.EndOneTimeCommandBuffer(commandBuffer);

        // Destroy the staging buffer and free its memory.
        VkDevice device = m_Context.GetDevice();
        vkFreeMemory(device, stagingDeviceMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
    }
}
