#version 460
#pragma shader_stage(vertex)

struct Instance {
    mat4 model;
    uint textureIndex;
};

layout(location = 0) in  vec3 i_Position;
layout(location = 1) in  vec2 i_TexCoord;

layout(location = 0)      out vec2 o_TexCoord;
layout(location = 1) flat out uint o_TextureIndex;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

layout(set = 0, binding = 2, std430) buffer StorageBuffer {
    Instance instances[];
};

void main() {
    gl_Position = mvp.projection * mvp.view * instances[gl_InstanceIndex].model * vec4(i_Position, 1.0);
    o_TextureIndex = instances[gl_InstanceIndex].textureIndex;
    o_TexCoord = i_TexCoord;
}

