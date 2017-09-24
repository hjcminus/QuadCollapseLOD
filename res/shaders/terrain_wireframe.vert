#version 430 core

layout (std140, binding = 0) uniform ubModelViewProj
{
	mat4	g_ModelViewProjectionMatrix;
};

layout (location = 0) in  vec3 vs_in_pos_local;

void main() {
	gl_Position = g_ModelViewProjectionMatrix * vec4(vs_in_pos_local, 1.0);
}
