#version 430 core

layout (binding = 0) uniform sampler2D g_Tex;

layout (location = 0) in  vec2	vs_out_texcoord;
layout (location = 0) out vec4	ps_out_color;

void main() {
	ps_out_color = texture(g_Tex, vs_out_texcoord);
}