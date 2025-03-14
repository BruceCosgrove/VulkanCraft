#pragma once

#include "Engine/Rendering/Image.hpp"
#include "Engine/Rendering/LocalTexture.hpp"

namespace eng
{
    struct Texture2DArrayInfo
    {
        RenderContext* RenderContext = nullptr;
        LocalTexture* LocalTexture = nullptr;
        VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
    };

    class Texture2DArray : public Image
    {
    public:
        Texture2DArray(Texture2DArrayInfo const& info);
    };
}
