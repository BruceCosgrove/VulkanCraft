#include "Texture2D.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"

namespace eng
{
    // TODO: make this more sophistocated/reliable.
    static VkFormat GetFormat(std::uint32_t channels)
    {
        ENG_ASSERT(0 < channels and channels <= 4);

        switch (channels)
        {
            case 1: return VK_FORMAT_R8_SRGB;
            case 2: return VK_FORMAT_R8G8_SRGB;
            case 3: return VK_FORMAT_R8G8B8_SRGB;
            case 4: return VK_FORMAT_R8G8B8A8_SRGB;
        }

        return VK_FORMAT_UNDEFINED;
    }

    Texture2D::Texture2D(Texture2DInfo const& info)
        : Image(info.RenderContext)
    {
        VkFormat format = GetFormat(info.LocalTexture->GetChannels());

        CreateImageAndAllocateMemory(*info.LocalTexture, format);
        CreateImageView(format);
        CreateSampler();
    }

    Texture2D::~Texture2D()
    {
        VkDevice device = m_Context.GetDevice();

        vkDestroySampler(device, m_Sampler, nullptr);
        vkDestroyImageView(device, m_ImageView, nullptr);
        vkFreeMemory(device, m_DeviceMemory, nullptr);
        vkDestroyImage(device, m_Image, nullptr);
    }

    VkImageView Texture2D::GetImageView() const
    {
        return m_ImageView;
    }

    VkSampler Texture2D::GetSampler() const
    {
        return m_Sampler;
    }

    void Texture2D::CreateImageAndAllocateMemory(LocalTexture2D& localTexture, VkFormat format)
    {
        // Create the staging buffer.
        VkDeviceSize stagingSize = localTexture.GetSize();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingDeviceMemory;

        Buffer::CreateBuffer(
            stagingSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingDeviceMemory
        );

        // Copy the texture data to the staging buffer.
        void* mappedMemory;
        Buffer::MapMemory(stagingDeviceMemory, 0, stagingSize, mappedMemory);
        std::memcpy(mappedMemory, localTexture.GetPixels().data(), stagingSize);
        Buffer::UnmapMemory(stagingDeviceMemory);

        // Create the image.
        VkExtent3D extent = {localTexture.GetWidth(), localTexture.GetHeight(), 1};

        Image::CreateImage(
            VK_IMAGE_TYPE_2D,
            format,
            extent,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_Image,
            m_DeviceMemory
        );

        VkCommandBuffer commandBuffer = m_Context.BeginOneTimeCommandBuffer();
        // Transition the image layout to be written to.
        Image::TransitionImageLayout(commandBuffer, m_Image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // Copy the texture from the staging buffer to the image.
        Image::CopyBufferToImage(commandBuffer, stagingBuffer, m_Image, extent);
        // Transition the image layout to be read from shaders.
        Image::TransitionImageLayout(commandBuffer, m_Image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_Context.EndOneTimeCommandBuffer(commandBuffer);

        // Destroy the staging buffer and free its memory.
        VkDevice device = m_Context.GetDevice();
        vkFreeMemory(device, stagingDeviceMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
    }

    void Texture2D::CreateImageView(VkFormat format)
    {
        VkDevice device = m_Context.GetDevice();

        VkImageViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = m_Image;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = format;
        //info.components // TODO: swizzle
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO
        info.subresourceRange.baseMipLevel = 0; // TODO
        info.subresourceRange.levelCount = 1; // TODO
        info.subresourceRange.baseArrayLayer = 0; // TODO
        info.subresourceRange.layerCount = 1; // TODO

        VkResult result = vkCreateImageView(device, &info, nullptr, &m_ImageView);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create texture image view.");
    }

    void Texture2D::CreateSampler()
    {
        auto& properties = RenderContext::GetPhysicalDeviceProperties();
        VkDevice device = m_Context.GetDevice();

        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_NEAREST; // TODO
        info.minFilter = VK_FILTER_NEAREST; // TODO
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // TODO, though this objectively looks better
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // TODO
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // TODO
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // TODO
        info.anisotropyEnable = VK_TRUE; // TODO: slider for performance, 0 (off) -> max
        info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // TODO
        info.unnormalizedCoordinates = VK_FALSE;

        VkResult result = vkCreateSampler(device, &info, nullptr, &m_Sampler);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create texture sampler.");
    }
}
