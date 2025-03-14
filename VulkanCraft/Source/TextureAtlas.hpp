#pragma once

#include <Engine.hpp>

namespace vc
{
    class TextureAtlas
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(TextureAtlas);

        TextureAtlas(
            eng::RenderContext& context,
            std::uint32_t textureSize,
            std::span<eng::LocalTexture> textures
        );
        ~TextureAtlas();

        VkSampler GetSampler() const;
        std::shared_ptr<eng::Texture2DArray> const& GetTexture() const;
        glm::uvec2 GetTextureCount() const;
        glm::vec2 GetTextureScale() const;
        std::uint32_t GetTexturesPerLayer() const;
        float GetTextureThreshold() const;

    private:
        void Stitch(std::span<eng::LocalTexture> textures);
        void CreateSampler();
    private:
        eng::RenderContext& m_Context; // non-owning

        VkSampler m_Sampler = VK_NULL_HANDLE;
        std::shared_ptr<eng::Texture2DArray> m_TextureAtlas;

        std::uint32_t m_TextureSize;
        glm::uvec2 m_TextureCount;
        glm::vec2 m_TextureScale;
        std::uint32_t m_TexturesPerLayer;
        float m_TextureThreshold;
    };
}
