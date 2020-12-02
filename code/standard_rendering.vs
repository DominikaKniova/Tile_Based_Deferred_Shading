#version 420 core

layout (location = 0) in vec4 a_Vertex;
layout (location = 1) in vec3 a_Normal;
layout (location = 3) in vec3 a_Translation;

uniform mat4  u_ModelViewMatrix;
uniform mat4  u_ProjectionMatrix;

out vec4 v_Vertex;
out vec3 v_Normal;

void main(void) {
    v_Vertex      = (a_Vertex + vec4(a_Translation, 0.0));
	v_Normal = normalize(a_Normal);
    gl_Position   = u_ProjectionMatrix * u_ModelViewMatrix * v_Vertex;
}
