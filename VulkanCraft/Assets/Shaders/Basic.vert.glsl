#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 i_Position;
layout(location = 1) in vec3 i_Color;

layout(location = 0) out vec3 o_Position;
layout(location = 1) out vec3 o_Color;

layout(std140, binding = 0) uniform _FrameData {
    mat4 ViewProjection;
} FrameData;

void main() {
    o_Position = i_Position;
    o_Color = i_Color;
    gl_Position = FrameData.ViewProjection * vec4(i_Position, 1);
}
