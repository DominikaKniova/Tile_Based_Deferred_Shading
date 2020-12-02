#version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform int maxNumLights;
uniform int superPixelSideSize;
uniform int numSuperPixels_x;
uniform sampler2D posTexture;
uniform sampler2D normTexture;

layout (binding = 0, r32ui) uniform uimage2D supPixelLightCounter;

struct Light {
	vec4 curr_pos_id;			
	vec4 original_pos_radius;		
	vec4 color_attenuation;
	vec4 radius;
};

layout (std140, binding = 0) uniform LightBlock {
    Light lights[512];
};

layout (binding = 1) buffer supPixelLights{
    uint supPixelLight[];
} Data;

void main() {
	//get coordinates of the super-pixel to which this fragment belong
	ivec2 supPixelCoords = ivec2(gl_FragCoord) / superPixelSideSize;

	// get number of lights that affect this super-pixel
	uint supPixelNumLights = imageLoad(supPixelLightCounter, supPixelCoords).x;

	// index of first affecting light in buffer of lights for this super-pixel
	int firstIdx = int(numSuperPixels_x * supPixelCoords.y * maxNumLights + supPixelCoords.x * maxNumLights);
	// index of last affecting light in buffer of lights for this super-pixel
	int lastIdx = firstIdx + int(supPixelNumLights) - 1;

	// get information stored in geometry pass
	vec3 pos = texture(posTexture, TexCoords).rgb;
    vec3 normal = normalize(texture(normTexture, TexCoords).rgb);

	// Phong lightning model
	vec3 phong = vec3(0.0);
	if (supPixelNumLights > 0){
		for (int i = firstIdx; i <= lastIdx; i ++){
			uint light_idx = Data.supPixelLight[i];
			vec3 light_pos = lights[light_idx].curr_pos_id.xyz;
			vec3 light_vec = light_pos - pos;
			float dist = length(light_vec);

			if (dist <= lights[light_idx].radius.x){
				float NdotL = max(0.0, dot(normalize(light_vec), normal));
				float att = 1.0 / (1.0 + 0.1*dist + lights[light_idx].color_attenuation.w*dist*dist);
				phong += att * lights[light_idx].color_attenuation.rgb * NdotL;
			}
		}
	}	

	FragColor = vec4(phong, 1.0);

}
	