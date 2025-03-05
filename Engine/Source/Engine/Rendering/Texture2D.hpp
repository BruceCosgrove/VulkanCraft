#pragma once

#include "Engine/Rendering/LocalTexture2D.hpp"
#include "Engine/Rendering/_Image.hpp"
#include <vulkan/vulkan.h>

namespace eng
{
    class RenderContext;

    struct Texture2DInfo
    {
        RenderContext* RenderContext = nullptr;
        LocalTexture2D* LocalTexture = nullptr;
    };

    class Texture2D : private detail::Image
    {
    public:
        Texture2D(Texture2DInfo const& info);
        ~Texture2D();

        VkImageView GetImageView() const;
        VkSampler GetSampler() const;
    private:
        void CreateImageAndAllocateMemory(LocalTexture2D& localTexture, VkFormat format);
        void CreateImageView(VkFormat format);
        void CreateSampler();
    private:
        VkImage m_Image = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkSampler m_Sampler = VK_NULL_HANDLE;
    };
}
