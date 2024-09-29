#version 460
#pragma shader_stage(fragment)

layout(location = 0)      in  vec2 i_TexCoord;
layout(location = 1) flat in  uint i_TextureIndex;
layout(location = 2) flat in  uint i_IsValid;

layout(location = 0) out vec4 o_Color;

layout(binding  = 1) uniform sampler2D u_Texture[32];

void main() {
    if(i_IsValid != 0) discard;
    o_Color = texture(u_Texture[i_TextureIndex], i_TexCoord);
}
