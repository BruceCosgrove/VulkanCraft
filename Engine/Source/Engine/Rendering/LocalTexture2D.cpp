#include "LocalTexture2D.hpp"
#include <stb_image.h>

namespace fs = std::filesystem;

namespace eng
{
    LocalTexture2D::LocalTexture2D(fs::path const& filepath)
    {
        m_Channels = STBI_rgb_alpha;
        m_Pixels.reset(stbi_load(filepath.string().c_str(), (int*)&m_Width, (int*)&m_Height, nullptr, m_Channels));
    }

    std::uint32_t LocalTexture2D::GetWidth() const
    {
        return m_Width;
    }

    std::uint32_t LocalTexture2D::GetHeight() const
    {
        return m_Height;
    }

    std::uint32_t LocalTexture2D::GetChannels() const
    {
        return m_Channels;
    }

    std::size_t LocalTexture2D::GetSize() const
    {
        return static_cast<std::size_t>(m_Width) * m_Height * m_Channels;
    }

    std::span<std::uint8_t const> LocalTexture2D::GetPixels() const
    {
        return {m_Pixels.get(), GetSize()};
    }
}
