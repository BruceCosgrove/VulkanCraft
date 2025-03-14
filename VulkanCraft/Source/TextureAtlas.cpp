#include "TextureAtlas.hpp"
#include <Engine/Core/AssertOrVerify.hpp>
#include <bit>

namespace vc
{
    TextureAtlas::TextureAtlas(
        eng::RenderContext& context,
        std::uint32_t textureSize,
        std::span<eng::LocalTexture> textures
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

    std::shared_ptr<eng::Texture2DArray> const& TextureAtlas::GetTexture() const
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

    std::uint32_t TextureAtlas::GetTexturesPerLayer() const
    {
        return m_TexturesPerLayer;
    }

    float TextureAtlas::GetTextureThreshold() const
    {
        return m_TextureThreshold;
    }

    void TextureAtlas::Stitch(std::span<eng::LocalTexture> textures)
    {
        auto& properties = m_Context.GetPhysicalDeviceProperties();
        std::uint32_t maxSize = properties.limits.maxImageDimension2D;
        std::uint32_t maxLayers = properties.limits.maxImageArrayLayers;

        std::uint32_t maxSize2 = maxSize * maxSize;
        std::uint32_t textureSize2 = m_TextureSize * m_TextureSize;
        std::uint32_t maxTexturesPerLayer = maxSize2 / textureSize2;

        std::uint32_t textureCount = static_cast<std::uint32_t>(textures.size());
        std::uint32_t atlasDepth = (textureCount + (maxTexturesPerLayer - 1)) / maxTexturesPerLayer;

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
        std::uint32_t atlasTier = std::countr_zero(std::bit_ceil(std::min(textureCount, maxTexturesPerLayer)));
        std::uint32_t textureCountX = (1 << (atlasTier >> 1)) * ((atlasTier & 1) + 1);
        std::uint32_t textureCountY = (1 << (atlasTier >> 1));

        std::uint32_t atlasWidth = textureCountX * m_TextureSize;
        std::uint32_t atlasHeight = textureCountY * m_TextureSize;
        std::uint32_t texturesPerLayer = textureCountX * textureCountY;

        ENG_LOG_INFO("Creating texture atlas of size {}x{}x{}.", atlasWidth, atlasHeight, atlasDepth);

        eng::LocalTexture atlas(atlasWidth, atlasHeight, atlasDepth);

        // TODO: I would use std::mdspan::submdspan, but msvc hasn't yet implemented that,
        // let alone fully implemented the multidimensional subscript operator.

        std::uint32_t textureRowSizeBytes = m_TextureSize * atlas.GetPixelSize();
        std::uint32_t atlasPixelRowSizeBytes = textureCountX * textureRowSizeBytes;
        std::uint32_t atlasRowSizeBytes = m_TextureSize * atlasPixelRowSizeBytes;
        std::uint32_t atlasSizeBytes = textureCountY * atlasRowSizeBytes;
        for (std::uint32_t textureZ = 0, textureIndex = 0; textureZ < atlasDepth; textureZ++)
        {
            std::uint32_t atlasLayer = textureZ * atlasSizeBytes;
            for (std::uint32_t textureY = 0; textureY < textureCountY; textureY++)
            {
                std::uint32_t atlasTop = textureY * atlasRowSizeBytes;
                for (std::uint32_t textureX = 0; textureX < textureCountX; textureX++, textureIndex++)
                {
                    std::uint32_t atlasLeft = textureX * textureRowSizeBytes;

                    auto& texture = textures[textureIndex];
                    std::uint8_t const* texturePixels = texture.GetPixels2D().data_handle();

                    std::uint8_t* atlasPixels = atlas.GetPixels3D().data_handle() + atlasLayer + atlasTop + atlasLeft;

                    for (std::uint32_t row = 0; row < m_TextureSize; row++)
                        std::memcpy(atlasPixels + atlasPixelRowSizeBytes * row, texturePixels + textureRowSizeBytes * row, textureRowSizeBytes);
                }
            }
        }

        eng::Texture2DArrayInfo info;
        info.RenderContext = &m_Context;
        info.LocalTexture = &atlas;
        info.Format = VK_FORMAT_R8G8B8A8_SRGB;
        m_TextureAtlas = std::make_shared<eng::Texture2DArray>(info);

        m_TextureCount = glm::uvec2(textureCountX, textureCountY);
        m_TextureScale = 1.0f / glm::vec2(textureCountX, textureCountY);
        m_TexturesPerLayer = textureCountX * textureCountY;
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
        info.anisotropyEnable = VK_TRUE; // TODO: slider for performance, 0 (off) -> max
        info.maxAnisotropy = eng::RenderContext::GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
        info.compareEnable = VK_FALSE;
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;

        VkResult result = vkCreateSampler(m_Context.GetDevice(), &info, nullptr, &m_Sampler);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create sampler.");
    }
}
