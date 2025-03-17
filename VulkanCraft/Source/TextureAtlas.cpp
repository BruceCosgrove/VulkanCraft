#include "TextureAtlas.hpp"
#include <Engine/Core/AssertOrVerify.hpp>
#include <bit>

namespace vc
{
    TextureAtlas::TextureAtlas(
        RenderContext& context,
        u32 textureSize,
        std::span<LocalTexture> textures
    )
        : m_Context(context)
        , m_TextureSize(textureSize)
    {
        Stitch(textures);
        CreateSampler();
    }

    TextureAtlas::~TextureAtlas()
    {
        vkDestroySampler(m_Context.GetDevice(), m_Sampler, nullptr);
    }

    VkSampler TextureAtlas::GetSampler() const
    {
        return m_Sampler;
    }

    std::shared_ptr<Texture2DArray> const& TextureAtlas::GetTexture() const
    {
        return m_TextureAtlas;
    }

    glm::uvec2 TextureAtlas::GetTextureCount() const
    {
        return m_TextureCount;
    }

    glm::vec2 TextureAtlas::GetTextureScale() const
    {
        return m_TextureScale;
    }

    u32 TextureAtlas::GetTexturesPerLayer() const
    {
        return m_TexturesPerLayer;
    }

    float TextureAtlas::GetTextureThreshold() const
    {
        return m_TextureThreshold;
    }

    void TextureAtlas::Stitch(std::span<LocalTexture> textures)
    {
        auto& properties = m_Context.GetPhysicalDeviceProperties();
        u32 maxSize = properties.limits.maxImageDimension2D;
        u32 maxLayers = properties.limits.maxImageArrayLayers;

        u32 textureCount = static_cast<u32>(textures.size());

        // Grow into the layers first, one at a time.
        // When all layers are used, double the texture size,
        // preferring x expansion, half the layer count,
        // then keep adding layers.
        // NOTE: since the texture size was doubled, it will
        // take twice as many textures to increase the layer
        // count as the previous texture size.

        // L = max Layers, C = texture Count, T = atlas Tier
        // C =  0L+1..  1L =>  1 x 1 x ceil(C/  1), T = 0
        // C =  1L+1..  2L =>  2 x 1 x ceil(C/  2), T = 1
        // C =  2L+1..  4L =>  2 x 2 x ceil(C/  4), T = 2
        // C =  4L+1..  8L =>  4 x 2 x ceil(C/  8), T = 3
        // C =  8L+1.. 16L =>  4 x 4 x ceil(C/ 16), T = 4
        // C = 16L+1.. 32L =>  8 x 4 x ceil(C/ 32), T = 5
        // C = 32L+1.. 64L =>  8 x 8 x ceil(C/ 64), T = 6
        // C = 64L+1..128L => 16 x 8 x ceil(C/128), T = 7

        // How many 1x1xL atlases would be needed to store all textures.
        u32 multiplesOfMaxlayer = (textureCount + maxLayers - 1) / maxLayers;
        // Describes the width and height of the atlas as per the table above.
        u32 atlasTier = std::countr_zero(std::bit_ceil(multiplesOfMaxlayer));

        u32 textureCountX = (1 << (atlasTier >> 1)) << (atlasTier & 1);
        u32 textureCountY = (1 << (atlasTier >> 1));
        u32 texturesPerLayer = textureCountX * textureCountY;
        u32 textureCountZ = (textureCount + texturesPerLayer - 1) / texturesPerLayer;

        u32 atlasWidth = textureCountX * m_TextureSize;
        u32 atlasHeight = textureCountY * m_TextureSize;
        u32 atlasDepth = textureCountZ;

        ENG_ASSERT(atlasWidth < maxSize and atlasHeight < maxSize, "Too many textures to store in atlas.");
        ENG_LOG_INFO("Creating texture atlas of size {}x{}x{}.", atlasWidth, atlasHeight, atlasDepth);

        LocalTexture atlas(atlasWidth, atlasHeight, atlasDepth);

        // TODO: I would use std::mdspan::submdspan, but msvc hasn't yet implemented that,
        // let alone fully implemented the multidimensional subscript operator.

        u32 textureRowSizeBytes = m_TextureSize * atlas.GetPixelSize();
        u32 atlasPixelRowSizeBytes = textureCountX * textureRowSizeBytes;
        u32 atlasRowSizeBytes = m_TextureSize * atlasPixelRowSizeBytes;
        u32 atlasSizeBytes = textureCountY * atlasRowSizeBytes;
        for (u32 textureZ = 0, textureIndex = 0; textureZ < atlasDepth; textureZ++)
        {
            u32 atlasLayer = textureZ * atlasSizeBytes;
            for (u32 textureY = 0; textureY < textureCountY; textureY++)
            {
                u32 atlasTop = textureY * atlasRowSizeBytes;
                for (u32 textureX = 0; textureX < textureCountX; textureX++, textureIndex++)
                {
                    u32 atlasLeft = textureX * textureRowSizeBytes;

                    if (textureIndex >= textures.size())
                        goto noMoreTextures;
                    u8 const* texturePixels = textures[textureIndex].GetPixels2D().data_handle();

                    u8* atlasPixels = atlas.GetPixels3D().data_handle() + atlasLayer + atlasTop + atlasLeft;

                    for (u32 row = 0; row < m_TextureSize; row++)
                        std::memcpy(atlasPixels + atlasPixelRowSizeBytes * row, texturePixels + textureRowSizeBytes * row, textureRowSizeBytes);
                }
            }
        }
    noMoreTextures:

        Texture2DArrayInfo info;
        info.RenderContext = &m_Context;
        info.LocalTexture = &atlas;
        info.Format = VK_FORMAT_R8G8B8A8_SRGB;
        m_TextureAtlas = std::make_shared<Texture2DArray>(info);

        m_TextureCount = uvec2(textureCountX, textureCountY);
        m_TextureScale = 1.0f / vec2(textureCountX, textureCountY);
        m_TexturesPerLayer = texturesPerLayer;
        m_TextureThreshold = 0.5f / m_TextureSize;
    }

    void TextureAtlas::CreateSampler()
    {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_NEAREST;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.anisotropyEnable = VK_FALSE;
        info.compareEnable = VK_FALSE;
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;

        VkResult result = vkCreateSampler(m_Context.GetDevice(), &info, nullptr, &m_Sampler);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create sampler.");
    }
}
