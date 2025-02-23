#pragma once

#include <string>
#include <vector>

namespace eng
{
    // Reads text from a file; returns true if the file was read, or false if not.
    // If it was read, contents contains the text of the file, otherwise contents is unmodified.
    bool ReadFile(std::string_view filepath, std::string& contents) noexcept;

    // Writes text to a file; returns true if the file was written, or false if not.
    bool WriteFile(std::string_view filepath, std::string const& contents) noexcept;

    // Reads bytes from a file; returns true if the file was read, or false if not.
    // If it was read, contents contains the bytes of the file, otherwise contents is unmodified.
    bool ReadBinaryFile(std::string_view filepath, std::vector<std::uint8_t>& contents) noexcept;

    // Writes bytes to a file; returns true if the file was written, or false if not.
    bool WriteBinaryFile(std::string_view filepath, std::vector<std::uint8_t> const& contents) noexcept;
}
