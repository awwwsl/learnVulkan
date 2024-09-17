#version 460
#pragma shader_stage(fragment)

layout(location = 0) in  vec2 i_TexCoord;
layout(location = 0) out vec4 o_Color;

void main() {
    o_Color = vec4(i_TexCoord, 0.0, 1.0);
}
