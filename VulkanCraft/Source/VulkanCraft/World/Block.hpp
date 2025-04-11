#pragma once

#include <Engine.hpp>

using namespace eng;

namespace vc
{
    enum class BlockID : u32 {};

    struct Block
    {
        small_string ID;
        // TODO: variants
    };
}
