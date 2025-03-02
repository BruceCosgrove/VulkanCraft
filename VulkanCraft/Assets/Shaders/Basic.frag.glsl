#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 i_Position;

layout(location = 0) out vec4 o_Color;

void main() {
    o_Color = vec4(mod(i_Position.xy + 1, 1), 0, 1);
}
