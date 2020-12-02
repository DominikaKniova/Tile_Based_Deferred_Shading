#version 420 core

out flat int instance_ID;

layout (location = 0) in vec3 a_Vertex;

uniform mat4  u_ModelViewMatrix;
uniform mat4  u_ProjectionMatrix;
uniform int lightID;

struct Light {
	vec4 curr_pos_id;			
	vec4 original_pos_radius;		
	vec4 color_attenuation;
	vec4 radius;
};

layout (std140, binding = 0) uniform LightBlock{
    Light lights[512];
} ;

void main(void) {
	vec3 o_Vertex = a_Vertex;
	o_Vertex *= lights[lightID].radius.x;
	o_Vertex += lights[lightID].curr_pos_id.xyz;
    gl_Position = u_ProjectionMatrix * u_ModelViewMatrix * vec4(o_Vertex, 1.0);
	instance_ID = lightID;
}
