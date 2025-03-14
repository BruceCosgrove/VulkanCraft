#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <filesystem>
#include <string>

namespace eng
{
    using i8 = std::int8_t;
    using u8 = std::uint8_t;
    using i16 = std::int16_t;
    using u16 = std::uint16_t;
    using i32 = std::int32_t;
    using u32 = std::uint32_t;
    using i64 = std::int64_t;
    using u64 = std::uint64_t;
    using f32 = float;
    using f64 = double;

    using ivec2 = glm::ivec2;
    using uvec2 = glm::uvec2;
    using vec2 = glm::vec2;
    using dvec2 = glm::dvec2;

    using ivec3 = glm::ivec3;
    using uvec3 = glm::uvec3;
    using vec3 = glm::vec3;
    using dvec3 = glm::dvec3;

    using ivec4 = glm::ivec4;
    using uvec4 = glm::uvec4;
    using vec4 = glm::vec4;
    using dvec4 = glm::dvec4;

    using mat2 = glm::mat2;
    using mat2x3 = glm::mat2x3;
    using mat2x4 = glm::mat2x4;

    using mat3x2 = glm::mat3x2;
    using mat3 = glm::mat3;
    using mat3x4 = glm::mat3x4;

    using mat4x2 = glm::mat4x2;
    using mat4x3 = glm::mat4x3;
    using mat4 = glm::mat4;

    using quat = glm::quat;
    using dquat = glm::dquat;

    // TODO: add that more optimized string class I made once it's done.
    using string = std::string;
    using string_view = std::string_view;

    using path = std::filesystem::path;
}
