#version 460
#pragma shader_stage(fragment)

layout(location = 0) in  vec2 i_TexCoord;

layout(location = 0) out vec4 o_Color;

// 0 = +X, 1 = -X, 2 = +Y, 3 = -Y, 4 = +Z, 5 = -Z
layout(binding  = 1) uniform sampler2D u_Texture[6];

layout(push_constant) uniform PushConstants {
  uint facing;
} pc;

void main() {
    gl_FragDepth = 1.0f;
    if(pc.facing == 2) // vulkan reverse y
    {
      o_Color = texture(u_Texture[3], i_TexCoord);
      return;
    }
    if(pc.facing == 3)
    {
      o_Color = texture(u_Texture[2], i_TexCoord);
      return;
    }
    o_Color = texture(u_Texture[pc.facing], i_TexCoord);
}
