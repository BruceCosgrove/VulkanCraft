#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in flat vec2 i_TexTopLeft;
layout(location = 2) in flat float i_TexLayer;

layout(location = 0) out vec4 o_Color;

// Frame data; uniform buffer; may change up to once per frame.
struct TextureAtlas {
    uvec3 TextureCount;     // Count, in number of textures, of the texture atlas.
    vec2 TextureScale;      // 1.0 / TextureCount.xy
    uint TexturesPerLayer;  // TextureCount.x * TextureCount.y
};
layout(std140, binding = 0) uniform _FrameData {
    mat4 ViewProjection;
    TextureAtlas BlockTextureAtlas;
} FrameData;

layout(binding = 2) uniform sampler2DArray BlockTextureAtlas;

vec3 CalculateTexCoord() {
    const vec2 scale = FrameData.BlockTextureAtlas.TextureScale;
    return vec3(i_TexTopLeft + fract(i_TexCoord) * scale, i_TexLayer);
}

void main() {
    const vec4 color = texture(BlockTextureAtlas, CalculateTexCoord());
    if (color.a == 0)
        discard;

    o_Color = color;
}
