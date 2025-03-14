#include "FileIO.hpp"
#include <fstream>
#include <sstream>

namespace eng
{
    bool ReadFile(path const& filepath, string& contents) noexcept
    {
        try
        {
            // Open the file.
            std::ifstream file(filepath, std::ios::ate);
            if (!file.is_open())
                return false;
            // Get the file size.
            std::streampos fileSize = file.tellg();
            if (fileSize < 0)
                return false;
            // Start the file at the beginning.
            if (!file.seekg(0, std::ios::beg))
                return false;
            // Read the entire file into the stream.
            std::stringstream stream;
            if (!(stream << file.rdbuf()))
                return false;
            // Move assign the stream's contents into the returned contents.
            contents = std::move(stream).str();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool WriteFile(path const& filepath, string const& contents) noexcept
    {
        try
        {
            // Open the file.
            std::ofstream file(filepath);
            if (!file.is_open())
                return false;
            // Write the contents to the file.
            return static_cast<bool>(file.write((char const*)contents.data(), std::ssize(contents)));
        }
        catch (...)
        {
            return false;
        }
    }

    bool ReadBinaryFile(path const& filepath, std::vector<u8>& contents) noexcept
    {
        try
        {
            // Open the file.
            std::ifstream file(filepath, std::ios::ate | std::ios::binary);
            if (!file.is_open())
                return false;
            // Get the file size.
            std::streampos fileSize = file.tellg();
            if (fileSize < 0)
                return false;
            // Start the file at the beginning.
            if (!file.seekg(0, std::ios::beg))
                return false;
            // Allocate enough space and set contents' size.
            contents.resize(static_cast<u64>(fileSize));
            // Read the entire file into contents.
            return static_cast<bool>(file.read((char*)contents.data(), fileSize));
        }
        catch (...)
        {
            return false;
        }
    }

    bool WriteBinaryFile(path const& filepath, std::vector<u8> const& contents) noexcept
    {
        try
        {
            // Open the file.
            std::ofstream file(filepath, std::ios::binary);
            if (!file.is_open())
                return false;
            // Read the entire file into the stream.
            return static_cast<bool>(file.write((char const*)contents.data(), std::ssize(contents)));
        }
        catch (...)
        {
            return false;
        }
    }
}
