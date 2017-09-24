#version 430 core

layout (std140, binding = 1) uniform ubFont
{
	vec4	g_FontColor;
};

layout (binding = 0) uniform sampler2D g_Tex;

layout (location = 0) in  vec2	vs_out_texcoord;
layout (location = 0) out vec4	ps_out_color;

void main() {
	vec4	tex_color = texture(g_Tex, vs_out_texcoord);
	ps_out_color = g_FontColor * tex_color;
	ps_out_color.a = tex_color.r;
}