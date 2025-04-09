#include "LocalTexture.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include <stb_image.h>

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

    LocalTexture::LocalTexture(path const& filepath)
        : m_Depth(1)
    {
        m_PixelSize = 4; // TODO
        m_Pixels.reset(stbi_load(filepath.string().c_str(), (i32*)&m_Width, (i32*)&m_Height, nullptr, STBI_rgb_alpha));
    }

    LocalTexture::LocalTexture(u32 width, u32 height)
        : m_PixelSize(4) // TODO
        , m_Width(width)
        , m_Height(height)
        , m_Depth(1)
        , m_Pixels(new u8[GetSize()])
    {

    }

    LocalTexture::LocalTexture(u32 width, u32 height, u32 depth)
        : m_PixelSize(4) // TODO
        , m_Width(width)
        , m_Height(height)
        , m_Depth(depth)
        , m_Pixels(new u8[GetSize()])
    {

    }

    u32 LocalTexture::GetPixelSize() const
    {
        return m_PixelSize;
    }

    u32 LocalTexture::GetWidth() const
    {
        return m_Width;
    }

    u32 LocalTexture::GetHeight() const
    {
        return m_Height;
    }

    u32 LocalTexture::GetDepth() const
    {
        return m_Depth;
    }

    u64 LocalTexture::GetSize() const
    {
        return u64(m_Depth) * m_Height * m_Width * m_PixelSize;
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
