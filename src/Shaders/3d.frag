#version 460
#pragma shader_stage(fragment)

struct Instance {
    mat4 model;
    uint textureIndex;
};

layout(location = 0)      in  vec2 i_TexCoord;
layout(location = 1) flat in  uint i_TextureIndex;

layout(location = 0) out vec4 o_Color;

layout(binding  = 1) uniform sampler2D u_Texture[32];

layout(set = 0, binding = 2, std430) buffer StorageBuffer {
    Instance instances[];
};

void main() {
    o_Color = texture(u_Texture[i_TextureIndex], i_TexCoord);
}
