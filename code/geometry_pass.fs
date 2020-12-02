#version 420 core

in vec4 v_Vertex;
in vec3 v_Normal;

layout (location = 0) out vec3 pos_texture;
layout (location = 1) out vec3 norm_texture;

void main() {
	pos_texture = v_Vertex.xyz;
	norm_texture =  v_Normal;
}
