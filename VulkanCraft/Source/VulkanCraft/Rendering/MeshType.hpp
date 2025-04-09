#pragma once

#include <Engine.hpp>

using namespace eng;

namespace vc
{
    ENG_DEFINE_BOUNDED_ENUM(
        MeshType, u8,

        // These six are for default block model faces.
        Left,
        Right,
        Bottom,
        Top,
        Back,
        Front,

        //Opaque, // e.g. flowers/crops/fire.

        //Translucent, // e.g. tinted glass
    );
}
