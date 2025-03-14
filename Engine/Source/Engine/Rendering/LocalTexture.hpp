#pragma once

#include "Engine/Core/DataTypes.hpp"
#include <filesystem>
#include <memory>
#include <mdspan>

namespace eng
{
    class LocalTexture
    {
    public:
        using pixels2 = std::mdspan<std::uint8_t, std::dextents<u32, 3>>;
        using const_pixels2 = std::mdspan<std::uint8_t const, std::dextents<u32, 3>>;
        using pixels3 = std::mdspan<std::uint8_t, std::dextents<u32, 4>>;
        using const_pixels3 = std::mdspan<std::uint8_t const, std::dextents<u32, 4>>;

        LocalTexture() = default;
        LocalTexture(LocalTexture&& texture) noexcept;
        LocalTexture& operator=(LocalTexture&& texture) noexcept;
        LocalTexture(path const& filepath);
        LocalTexture(u32 width, u32 height);
        LocalTexture(u32 width, u32 height, u32 depth);

        u32 GetPixelSize() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        u32 GetDepth() const;
        u64 GetSize() const;

        pixels2 GetPixels2D();
        const_pixels2 GetPixels2D() const;
        pixels3 GetPixels3D();
        const_pixels3 GetPixels3D() const;
    private:
        u32 m_PixelSize = 0;
        u32 m_Width = 0;
        u32 m_Height = 0;
        u32 m_Depth = 0;
        std::unique_ptr<u8> m_Pixels;
    };
}
