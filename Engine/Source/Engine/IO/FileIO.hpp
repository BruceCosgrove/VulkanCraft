#pragma once

#include "Engine/Core/DataTypes.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace eng
{
    // Reads text from a file; returns true if the file was read, or false if not.
    // If it was read, contents contains the text of the file, otherwise contents is unmodified.
    bool ReadFile(path const& filepath, string& contents) noexcept;

    // Writes text to a file; returns true if the file was written, or false if not.
    bool WriteFile(path const& filepath, string const& contents) noexcept;

    // Reads bytes from a file; returns true if the file was read, or false if not.
    // If it was read, contents contains the bytes of the file, otherwise contents is unmodified.
    bool ReadBinaryFile(path const& filepath, std::vector<u8>& contents) noexcept;

    // Writes bytes to a file; returns true if the file was written, or false if not.
    bool WriteBinaryFile(path const& filepath, std::vector<u8> const& contents) noexcept;
}
