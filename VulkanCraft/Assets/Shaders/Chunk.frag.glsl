#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in flat vec2 i_TexTopLeft;
layout(location = 2) in flat vec2 i_TexScale;
layout(location = 3) in flat float i_TexLayer;

layout(location = 0) out vec4 o_Color;

layout(binding = 2) uniform sampler2DArray BlockTextureAtlas;

vec3 CalculateTexCoord() {
    return vec3(i_TexTopLeft + mod(i_TexCoord, 1) * i_TexScale, i_TexLayer);
}

void main() {
    const vec4 color = texture(BlockTextureAtlas, CalculateTexCoord());
    if (color.a == 0)
        discard;

    o_Color = color;
}
