#version 460
#pragma shader_stage(vertex)

layout(location = 0) in  vec3 i_Position;
layout(location = 1) in  vec2 i_TexCoord;

layout(location = 0) out vec2 o_TexCoord;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

void main() {
    gl_Position = mvp.projection * mvp.view * mvp.model * vec4(i_Position, 1.0);
    o_TexCoord =  i_TexCoord;
}

