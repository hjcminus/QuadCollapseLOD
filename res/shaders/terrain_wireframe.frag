#version 430 core

layout (std140, binding = 1) uniform ubWireframe
{
	vec4	g_WireframeColor;
};

layout (location = 0) out vec4 ps_out_color;

void main() {
	ps_out_color = g_WireframeColor;
}