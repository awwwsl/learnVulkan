#version 460
#pragma shader_stage(vertex)

layout(location = 0) in  vec3 i_Position;

layout(location = 0) out vec4 o_Color;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

layout(push_constant, std140) uniform PushConstant {
    mat4 model;
    vec4 color;
} pc;

void main() {
  gl_Position = mvp.projection * mvp.view * pc.model * vec4(i_Position, 1.0);
  o_Color = pc.color;
}

