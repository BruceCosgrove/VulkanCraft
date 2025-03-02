#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in uvec2 i_PackedFaceData;

layout(location = 0) out vec2 o_TexCoord;
layout(location = 1) out flat vec2 o_TexTopLeft;
layout(location = 2) out flat vec2 o_TexScale;
layout(location = 3) out flat float o_TexLayer;

// Frame data; uniform buffer; may change up to once per frame.
layout(std140, binding = 0) uniform _FrameData {
    mat4 ViewProjection;
    
    uint BlockTextureSize; // Always a power of 2; default resource pack is 16.
    uvec2 BlockTextureAtlasBlockSize; // Always a power of 2. Size, in number of block textures, of the block texture atlas.
} FrameData;

// Chunk data; storage buffer; changes every time the player moves between two chunks.
layout(std140, binding = 1) readonly buffer _ChunkData {
    uvec2 PackedChunkData[];
} ChunkData;

// Unpacking data.

void UnpackFaceData(
    const uvec2 packedFaceData,
    out uvec3 localPosition,    // 0..15
    out uvec2 size,             // 1..16
    out uint textureID          // only 16 bits
) {
    //                   packedFaceData.y                 packedFaceData.x
    // 64  60  56  52  48  44  40  36  32   28  24  20  16  12   8   4   0
    //   ----------------------------hhhh wwwwzzzzyyyyxxxxtttttttttttttttt
    // x = local x, y = local y, z = local z, w = width, h = height
    localPosition = 0xF & uvec3(packedFaceData.x >> 16, packedFaceData.x >> 20, packedFaceData.x >> 24);
    size = 1 + (0xF & uvec2(packedFaceData.x >> 28, packedFaceData.y));
    textureID = 0xFFFF & packedFaceData.x;
}

void UnpackChunkData(
    const uvec2 packedChunkData,
    out ivec3 relativeChunkPosition, // only 16 bits (per channel)
    out uint face                    // 0, 1, 2, 3, 4, 5 => left(-x), right(+x), bottom(-y), top(+y), back(-z), front(+z)
) {
    //                  packedChunkData.y                packedChunkData.x
    // 64  60  56  52  48  44  40  36  32   28  24  20  16  12   8   4   0
    //   -------------fffzzzzzzzzzzzzzzzz yyyyyyyyyyyyyyyyxxxxxxxxxxxxxxxx
    // x = chunk x, y = chunk y, z = chunk z, f = face
    relativeChunkPosition = 0xFFFF & ivec3(packedChunkData.x, packedChunkData.x >> 16, packedChunkData.y);
    face = 0x7 & (packedChunkData.y >> 16);
}

// Bake the super simple instance data in the shader (344 bytes).

const uint s_InstanceIndices[6] = {
    0, 1, 2, 2, 3, 0
};

const vec2 s_InstanceTexCoords[4] = {
    {0, 1}, // bottom left
    {1, 1}, // bottom right
    {1, 0}, // top    right
    {0, 0}, // top    left
};

const vec3 s_InstancePositions[6][4] = {
    // Left
    {
        {0, 0, 0}, // bottom left
        {0, 0, 1}, // bottom right
        {0, 1, 1}, // top    right
        {0, 1, 0}, // top    left
    },
    // Right
    {
        {1, 0, 1}, // bottom left
        {1, 0, 0}, // bottom right
        {1, 1, 0}, // top    right
        {1, 1, 1}, // top    left
    },
    // Bottom
    {
        {0, 0, 0}, // bottom left
        {1, 0, 0}, // bottom right
        {1, 0, 1}, // top    right
        {0, 0, 1}, // top    left
    },
    // Top
    {
        {0, 1, 1}, // bottom left
        {1, 1, 1}, // bottom right
        {1, 1, 0}, // top    right
        {0, 1, 0}, // top    left
    },
    // Back
    {
        {1, 0, 0}, // bottom left
        {0, 0, 0}, // bottom right
        {0, 1, 0}, // top    right
        {1, 1, 0}, // top    left
    },
    // Front
    {
        {0, 0, 1}, // bottom left
        {1, 0, 1}, // bottom right
        {1, 1, 1}, // top    right
        {0, 1, 1}, // top    left
    },
};

void main() {
    // Unpack all the data.

    uvec3 localPosition;
    uvec2 size;
    uint textureID;
    ivec3 chunkPosition;
    uint face;

    UnpackFaceData(i_PackedFaceData, localPosition, size, textureID);
    UnpackChunkData(ChunkData.PackedChunkData[gl_DrawID], chunkPosition, face);

    // Calculate texture coordinates based on the textureID.

    // TODO: precompute these two out of the shader and send them here via FrameData.
    const vec2 textureScale = 1.0 / vec2(FrameData.BlockTextureAtlasBlockSize);
    const uint texturesPerAtlasLayer = FrameData.BlockTextureAtlasBlockSize.x * FrameData.BlockTextureAtlasBlockSize.y;

    const uint textureX = textureID % FrameData.BlockTextureAtlasBlockSize.x;
    const uint textureY = (textureID % texturesPerAtlasLayer) / FrameData.BlockTextureAtlasBlockSize.y;
    const uint textureZ = textureID / texturesPerAtlasLayer;

    // Change which axes get scaled depending on the face direction.
    const uvec3 sizes[3] = {
        uvec3(1, size),           // left/right face
        uvec3(size.x, 1, size.y), // bottom/top face
        uvec3(size, 1),           // back/front face
    };
    const uvec3 size3 = sizes[face >> 1]; // left/right = 0, bottom/top = 1, back/front = 2

    // Index into the instance data.
    const uint index = s_InstanceIndices[gl_VertexIndex];

    const vec3 position = s_InstancePositions[face][index] * size3 + vec3(localPosition) + chunkPosition * 16;
    const vec2 texCoord = s_InstanceTexCoords[index] * size;
    const vec2 texTopLeft = vec2(textureX, textureY) * textureScale;
    const vec2 texScale = textureScale;
    const float texLayer = float(textureZ);

    o_TexCoord = texCoord;
    o_TexTopLeft = texTopLeft;
    o_TexScale = texScale;
    o_TexLayer = texLayer;
    gl_Position = FrameData.ViewProjection * vec4(position, 1);
}
