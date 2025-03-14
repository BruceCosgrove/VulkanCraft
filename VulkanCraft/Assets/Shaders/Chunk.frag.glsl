#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in flat vec2 i_TexTopLeft;
layout(location = 2) in flat float i_TexLayer;

layout(location = 0) out vec4 o_Color;

// Frame data; uniform buffer; may change up to once per frame.
struct TextureAtlas {
    uvec2 TextureCount;     // Count, in number of textures, of the texture atlas.
    vec2 TextureScale;      // 1.0 / TextureCount
    uint TexturesPerLayer;  // TextureCount.x * TextureCount.y
    float TextureThreshold; // 0.5 / (size of a single texture in pixels); Texture bleeding removal. NOTE: not the usual texture bleeding. The stuff this fixes is caused by modulated texture coordinates in the fragment shader, namely mod(1.0, 1.0), resulting in a texture coordinate of 0.0.
};
layout(std140, binding = 0) uniform _FrameData {
    mat4 ViewProjection;
    TextureAtlas BlockTextureAtlas;
} FrameData;

layout(binding = 2) uniform sampler2DArray BlockTextureAtlas;

vec3 CalculateTexCoord() {
    const float threshold = FrameData.BlockTextureAtlas.TextureThreshold;
    const vec2 scale = FrameData.BlockTextureAtlas.TextureScale;
    const vec2 localTexCoord = mod(min(max(i_TexCoord, threshold), 1 - threshold), 1);
    return vec3(i_TexTopLeft + localTexCoord * scale, i_TexLayer);
}

void main() {
    const vec4 color = texture(BlockTextureAtlas, CalculateTexCoord());
    if (color.a == 0)
        discard;

    o_Color = color;
}
