#version 430 core

layout (std140, binding = 0) uniform ubModelViewProj
{
	mat4	g_ModelViewProjectionMatrix;
};

layout (location = 0) in  vec3	vs_in_pos_local;
layout (location = 1) in  vec2	vs_in_texcoord;

layout (location = 0) out vec2	vs_out_texcoord;

void main() {
	gl_Position = g_ModelViewProjectionMatrix * vec4(vs_in_pos_local, 1.0);
	vs_out_texcoord = vs_in_texcoord; // pass though
}
