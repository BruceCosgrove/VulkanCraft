#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 i_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(binding = 0) uniform sampler2D Texture;

void main() {
    o_Color = texture(Texture, i_TexCoord);
}
