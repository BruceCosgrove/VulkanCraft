#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec2 i_Position;

layout (location = 0) out vec4 o_Color;

void main() {
    o_Color = vec4(i_Position + 0.5, 0, 1);
}
