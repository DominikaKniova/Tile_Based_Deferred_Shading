#version 420 core

in vec4 v_Vertex;
in vec3 v_Normal;
layout (location = 0) out vec4 FragColor;

uniform int num_Lights;

struct Light {
	vec4 curr_pos_id;			
	vec4 original_pos_radius;		
	vec4 color_attenuation;
	vec4 radius;
};

layout (std140, binding = 0) uniform LightBlock{
    Light lights[512];
} ;

void main() {
	// Phong lightning model
	vec3 phong = vec3(0.0);
	for(int i = 0; i < num_Lights; i++){
		vec3 light_pos = lights[i].curr_pos_id.xyz;
		float dist = length(light_pos - v_Vertex.xyz);
		if (dist <= lights[i].radius.x){
			float NdotL = max(0.0, dot(normalize(light_pos - v_Vertex.xyz), normalize(v_Normal)));
			float att = 1.0 / (1.0 + 0.1*dist + lights[i].color_attenuation.w*dist*dist);
			phong += att * lights[i].color_attenuation.rgb * NdotL;
		}		
	}
	FragColor   = vec4(phong, 1.0);
}
