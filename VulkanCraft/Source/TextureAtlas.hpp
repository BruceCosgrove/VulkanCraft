#pragma once

#include <Engine.hpp>

using namespace eng;

namespace vc
{
    class TextureAtlas
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(TextureAtlas);

        TextureAtlas(
            RenderContext& context,
            u32 textureSize,
            std::span<LocalTexture> textures
        );
        ~TextureAtlas();

        VkSampler GetSampler() const;
        std::shared_ptr<Texture2DArray> const& GetTexture() const;
        uvec2 GetTextureCount() const;
        vec2 GetTextureScale() const;
        u32 GetTexturesPerLayer() const;
        float GetTextureThreshold() const;

    private:
        void Stitch(std::span<LocalTexture> textures);
        void CreateSampler();
    private:
        RenderContext& m_Context; // non-owning

        VkSampler m_Sampler = VK_NULL_HANDLE;
        std::shared_ptr<Texture2DArray> m_TextureAtlas;

        u32 m_TextureSize;
        uvec2 m_TextureCount;
        vec2 m_TextureScale;
        u32 m_TexturesPerLayer;
        f32 m_TextureThreshold;
    };
}
