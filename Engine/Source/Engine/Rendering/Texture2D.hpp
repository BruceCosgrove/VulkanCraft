#pragma once

#include "Engine/Rendering/Image.hpp"
#include "Engine/Rendering/LocalTexture.hpp"

namespace eng
{
    struct Texture2DInfo
    {
        RenderContext* RenderContext = nullptr;
        LocalTexture* LocalTexture = nullptr;
        VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
    };

    class Texture2D : public Image
    {
    public:
        Texture2D(Texture2DInfo const& info);
    };
}
