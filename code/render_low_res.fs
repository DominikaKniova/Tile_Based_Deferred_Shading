#version 420 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D pos_texture;

void main() {
	vec3 pos = texture(pos_texture, TexCoords).rgb;
	FragColor   = vec4(pos , 1.0);
}
