#version 460
#pragma shader_stage(fragment)

layout(location = 0) in  vec2 i_TexCoord;
layout(location = 0) out vec4 o_Color;

// 0 = +X, 1 = -X, 2 = +Y, 3 = -Y, 4 = +Z, 5 = -Z
layout(binding  = 1) uniform sampler2D u_Texture1;
layout(binding  = 2) uniform sampler2D u_Texture2;
layout(binding  = 3) uniform sampler2D u_Texture3;
layout(binding  = 4) uniform sampler2D u_Texture4;
layout(binding  = 5) uniform sampler2D u_Texture5;
layout(binding  = 6) uniform sampler2D u_Texture6;

layout(push_constant, std140) uniform PushConstant {
    uint facing;
} pc;


void main() {
    gl_FragDepth = 1.0f;
    if(pc.facing == 0)
    {
      o_Color = texture(u_Texture1, i_TexCoord);
      return;
    }
    if(pc.facing == 1)
    {
      o_Color = texture(u_Texture2, i_TexCoord);
      return;
    }
    if(pc.facing == 3) // vulkan reverse y
    {
      o_Color = texture(u_Texture3, i_TexCoord);
      return;
    }
    if(pc.facing == 2)
    {
      o_Color = texture(u_Texture4, i_TexCoord);
      return;
    }
    if(pc.facing == 4)
    {
      o_Color = texture(u_Texture5, i_TexCoord);
      return;
    }
    if(pc.facing == 5)
    {
      o_Color = texture(u_Texture6, i_TexCoord);
      return;
    }
}
