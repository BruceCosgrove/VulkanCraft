#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec2 o_TexCoord;

const vec4 s_InstanceVertices[] = {
    {-1, -1, 0, 1}, // bottom left
    {+1, -1, 1, 1}, // bottom right
    {+1, +1, 1, 0}, // top    left
    {-1, +1, 0, 0}, // top    right
};

void main() {
    const vec4 vertex = s_InstanceVertices[gl_VertexIndex];
    o_TexCoord = vertex.zw;
    gl_Position = vec4(vertex.xy, 0, 1);
}
