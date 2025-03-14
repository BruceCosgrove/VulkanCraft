#pragma once

#include <filesystem>
#include <memory>
#include <mdspan>

namespace eng
{
    class LocalTexture
    {
    public:
        using pixels2 = std::mdspan<std::uint8_t, std::dextents<std::uint32_t, 3>>;
        using const_pixels2 = std::mdspan<std::uint8_t const, std::dextents<std::uint32_t, 3>>;
        using pixels3 = std::mdspan<std::uint8_t, std::dextents<std::uint32_t, 4>>;
        using const_pixels3 = std::mdspan<std::uint8_t const, std::dextents<std::uint32_t, 4>>;

        LocalTexture() = default;
        LocalTexture(LocalTexture&& texture) noexcept;
        LocalTexture& operator=(LocalTexture&& texture) noexcept;
        LocalTexture(std::filesystem::path const& filepath);
        LocalTexture(std::uint32_t width, std::uint32_t height);
        LocalTexture(std::uint32_t width, std::uint32_t height, std::uint32_t depth);

        std::uint32_t GetPixelSize() const;
        std::uint32_t GetWidth() const;
        std::uint32_t GetHeight() const;
        std::uint32_t GetDepth() const;
        std::uint64_t GetSize() const;

        pixels2 GetPixels2D();
        const_pixels2 GetPixels2D() const;
        pixels3 GetPixels3D();
        const_pixels3 GetPixels3D() const;
    private:
        std::uint32_t m_PixelSize = 0;
        std::uint32_t m_Width = 0;
        std::uint32_t m_Height = 0;
        std::uint32_t m_Depth = 0;
        std::unique_ptr<std::uint8_t> m_Pixels;
    };
}
