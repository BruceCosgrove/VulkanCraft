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

        u32 maxSize2 = maxSize * maxSize;
        u32 textureSize2 = m_TextureSize * m_TextureSize;
        u32 maxTexturesPerLayer = maxSize2 / textureSize2;

        u32 textureCount = static_cast<u32>(textures.size());
        u32 atlasDepth = (textureCount + (maxTexturesPerLayer - 1)) / maxTexturesPerLayer;

        // TODO: to save memory with larger and/or more textures,
        // when textureCount > maxTexturesPerLayer, instead of allocating a new max size layer,
        // reduce the size of each layer and increase the number of layers.
        // e.g. maxSize * maxSize * 2 > maxSize * maxsize/2 * 3
        // 
        // Minimum allocation alignment size should be considered only when
        // sizeof(pixel in texture) * textureSize < min allocation alignment

        // textureCount tier cx cy
        // 1            0     1x 1
        // 2            1     2x 1
        // 3..4         2     2x 2
        // 5..8         3     4x 2
        // 9..16        4     4x 4
        // 17..32       5     8x 4
        // 33..64       6     8x 8
        // 65..128      7    16x 8
        // 129..256     8    16x16
        u32 atlasTier = std::countr_zero(std::bit_ceil(std::min(textureCount, maxTexturesPerLayer)));
        u32 textureCountX = (1 << (atlasTier >> 1)) * ((atlasTier & 1) + 1);
        u32 textureCountY = (1 << (atlasTier >> 1));

        u32 atlasWidth = textureCountX * m_TextureSize;
        u32 atlasHeight = textureCountY * m_TextureSize;
        u32 texturesPerLayer = textureCountX * textureCountY;

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
                    auto& texture = textures[textureIndex];
                    u8 const* texturePixels = texture.GetPixels2D().data_handle();

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
