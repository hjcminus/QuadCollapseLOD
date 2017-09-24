#version 430 core

layout (std140, binding = 2) uniform ubFog
{
	vec3	g_FogColor;
	float	g_FogDensity;
};

layout (binding = 0) uniform sampler2D g_TexBase;
layout (binding = 1) uniform sampler2D g_TexDetail;

layout (location = 0) in  vec3 vs_out_pos_view;
layout (location = 1) in  vec2 vs_out_texcoord;
layout (location = 0) out vec4 ps_out_color;

const float GRADIENT = 5.0;

void main() {
	vec4 base_color = texture(g_TexBase, vs_out_texcoord);
	vec4 detail_color = texture(g_TexDetail, vs_out_texcoord * 64.0);
	vec4 scene_color = mix(base_color, detail_color, 0.5);
	
	float distance = length(vs_out_pos_view);
    float visibility = exp(-pow((distance * g_FogDensity), GRADIENT));
    visibility = clamp(visibility, 0.0, 1.0);
    float a = 1.0 - visibility;
	
	ps_out_color = mix(scene_color, vec4(g_FogColor, 1.0), a);
}