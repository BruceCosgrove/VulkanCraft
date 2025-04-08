#pragma once

#include <Engine.hpp>

using namespace eng;

namespace vc
{
    ENG_DEFINE_BOUNDED_ENUM(
        ChunkGenerationStage, u8,

        Terrain, // TODO: different terrain stages, e.g. stone noise map, caves, foliage, etc...
        Mesh,
    );
}
