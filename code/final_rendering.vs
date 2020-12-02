#version 420 core

layout (location = 0) in vec3 a_Vertex;
layout (location = 1) in vec2 a_TexCoords;

uniform mat4  u_ModelViewMatrix;
uniform mat4  u_ProjectionMatrix;

out vec2 TexCoords;

void main(void) {
    // Transform vertex into view-space
    gl_Position   = vec4(a_Vertex, 1.0);
	TexCoords = a_TexCoords;
}
