#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 i_Position;
layout(location = 1) in vec3 i_Color;
layout(location = 2) in vec2 i_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(binding = 1) uniform sampler2D Texture;

void main() {
    o_Color = vec4(texture(Texture, i_TexCoord).rgb * i_Color, 1);
}
