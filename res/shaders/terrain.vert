#version 430 core

layout (std140, binding = 0) uniform ubModelViewProj
{
	mat4	g_ModelViewProjectionMatrix;
	mat4	g_ModelViewMatrix;
};

layout (std140, binding = 1) uniform ubHeightField
{
	vec4	g_HeightFieldSize;
};

layout (location = 0) in  vec3 vs_in_pos_local;
layout (location = 0) out vec3 vs_out_pos_view;
layout (location = 1) out vec2 vs_out_texcoord;

void main() {
	vec4 v4 = vec4(vs_in_pos_local, 1.0);
	gl_Position = g_ModelViewProjectionMatrix * v4;
	vs_out_pos_view = (g_ModelViewMatrix * v4).xyz;
	vs_out_texcoord.x = vs_in_pos_local.x / g_HeightFieldSize.x;
	vs_out_texcoord.y = vs_in_pos_local.y / g_HeightFieldSize.x;
}
