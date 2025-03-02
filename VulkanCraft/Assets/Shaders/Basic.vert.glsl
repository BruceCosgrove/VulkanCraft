#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 i_Position;

layout(location = 0) out vec3 o_Position;

void main() {
    o_Position = i_Position;
    gl_Position = vec4(i_Position, 1);
}
