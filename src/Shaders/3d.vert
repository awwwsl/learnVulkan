#version 460
#pragma shader_stage(vertex)

struct Instance {
    mat4 model;
    uint textureIndex;
    bool isValid;
};

layout(location = 0) in  vec3 i_Position;
layout(location = 1) in  vec2 i_TexCoord;

layout(location = 0)      out vec2 o_TexCoord;
layout(location = 1) flat out uint o_TextureIndex;
layout(location = 2) flat out uint o_IsValid;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

layout(set = 0, binding = 2, std430) buffer StorageBuffer {
    Instance instances[];
} storageBuffer[4096];

layout(push_constant) uniform PushConstants {
  uint storageBufferIndex;
} pc;

void main() { // HACK: 4096 -> src/Models/chunk.cpp:chunkSize
    if(!storageBuffer[pc.storageBufferIndex].instances[gl_InstanceIndex].isValid) {
        o_IsValid = 1;
        return;
    }
    gl_Position = mvp.projection * mvp.view * storageBuffer[pc.storageBufferIndex].instances[gl_InstanceIndex].model * vec4(i_Position, 1.0);
    o_TextureIndex = storageBuffer[pc.storageBufferIndex].instances[gl_InstanceIndex].textureIndex;
    o_TexCoord = i_TexCoord;
    o_IsValid = 0;
}

