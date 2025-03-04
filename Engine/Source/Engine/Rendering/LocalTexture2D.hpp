#pragma once

#include <filesystem>
#include <memory>
#include <span>

namespace eng
{
    class LocalTexture2D
    {
    public:
        LocalTexture2D(std::filesystem::path const& filepath);

        std::uint32_t GetWidth() const;
        std::uint32_t GetHeight() const;
        std::uint32_t GetChannels() const;
        std::size_t GetSize() const;
        std::span<std::uint8_t const> GetPixels() const;
    private:
        std::uint32_t m_Width = 0;
        std::uint32_t m_Height = 0;
        std::uint32_t m_Channels = 0;
        std::unique_ptr<std::uint8_t> m_Pixels;
    };
}
