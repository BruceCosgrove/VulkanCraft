#include "LocalTexture.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include <stb_image.h>

namespace fs = std::filesystem;

namespace eng
{
    LocalTexture::LocalTexture(LocalTexture&& texture) noexcept
        : m_PixelSize(std::exchange(texture.m_PixelSize, 0))
        , m_Width(std::exchange(texture.m_Width, 0))
        , m_Height(std::exchange(texture.m_Height, 0))
        , m_Depth(std::exchange(texture.m_Depth, 0))
        , m_Pixels(std::exchange(texture.m_Pixels, nullptr))
    {

    }

    LocalTexture& LocalTexture::operator=(LocalTexture&& texture) noexcept
    {
        if (this != std::addressof(texture))
        {
            m_PixelSize = std::exchange(texture.m_PixelSize, 0);
            m_Width = std::exchange(texture.m_Width, 0);
            m_Height = std::exchange(texture.m_Height, 0);
            m_Depth = std::exchange(texture.m_Depth, 0);
            m_Pixels = std::exchange(texture.m_Pixels, nullptr);
        }
        return *this;
    }

    LocalTexture::LocalTexture(fs::path const& filepath)
        : m_Depth(1)
    {
        m_PixelSize = 4; // TODO
        m_Pixels.reset(stbi_load(filepath.string().c_str(), (int*)&m_Width, (int*)&m_Height, nullptr, STBI_rgb_alpha));
    }

    LocalTexture::LocalTexture(std::uint32_t width, std::uint32_t height)
        : m_PixelSize(4) // TODO
        , m_Width(width)
        , m_Height(height)
        , m_Depth(1)
        , m_Pixels(new std::uint8_t[GetSize()])
    {

    }

    LocalTexture::LocalTexture(std::uint32_t width, std::uint32_t height, std::uint32_t depth)
        : m_PixelSize(4) // TODO
        , m_Width(width)
        , m_Height(height)
        , m_Depth(depth)
        , m_Pixels(new std::uint8_t[GetSize()])
    {

    }

    std::uint32_t LocalTexture::GetPixelSize() const
    {
        return m_PixelSize;
    }

    std::uint32_t LocalTexture::GetWidth() const
    {
        return m_Width;
    }

    std::uint32_t LocalTexture::GetHeight() const
    {
        return m_Height;
    }

    std::uint32_t LocalTexture::GetDepth() const
    {
        return m_Depth;
    }

    std::uint64_t LocalTexture::GetSize() const
    {
        return static_cast<std::uint64_t>(m_Depth) * m_Height * m_Width * m_PixelSize;
    }

    auto LocalTexture::GetPixels2D() -> pixels2
    {
        return pixels2(m_Pixels.get(), m_Height, m_Width, m_PixelSize);
    }

    auto LocalTexture::GetPixels2D() const -> const_pixels2
    {
        return const_pixels2(m_Pixels.get(), m_Height, m_Width, m_PixelSize);
    }

    auto LocalTexture::GetPixels3D() -> pixels3
    {
        return pixels3(m_Pixels.get(), m_Depth, m_Height, m_Width, m_PixelSize);
    }

    auto LocalTexture::GetPixels3D() const -> const_pixels3
    {
        return pixels3(m_Pixels.get(), m_Depth, m_Height, m_Width, m_PixelSize);
    }
}
