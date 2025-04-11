#pragma once

#include <Engine.hpp>

using namespace eng;

namespace vc
{
    ENG_DEFINE_BOUNDED_ENUM(
        ChunkGenerationStage, u8,

        StoneMap,   // Initial stone map.
        Topsoil,    // Replace some top layers of stone with dirt/sandstone/biome-specific blocks.
        //Caves,      // Caves.
        Surface,    // Replace fewer top layers of topsoil with grass/sand/bione-specific blocks.
        //Structures, // Villages, sea temples, mineshafts, etc.
        //Features,   // Trees, cacti, stumps/fallen trees?
        //Foliage,    // Flowers, short/tall grass, other small blocks.

        Mesh,       // A renderable mesh once world generation is complete.
    );
}
