#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) out vec2 o_Position;

const vec2 s_Positions[] = vec2[](
    vec2(-0.5, -0.5),
    vec2(+0.5, -0.5),
    vec2(+0.5, +0.5),
    vec2(+0.5, +0.5),
    vec2(-0.5, +0.5),
    vec2(-0.5, -0.5)
);

void main() {
    o_Position = s_Positions[gl_VertexIndex];
    gl_Position = vec4(s_Positions[gl_VertexIndex], 0, 1);
}
