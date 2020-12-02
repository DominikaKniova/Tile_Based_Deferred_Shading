#version 430 core

in flat int instance_ID;
out vec4 FragColor;

layout (binding = 0, r32ui) uniform uimage2D supPixelLightCounter; 
uniform int numSuperPixels_x;
uniform int maxNumLights;

struct Light {
	vec4 curr_pos_id;			
	vec4 original_pos_radius;		
	vec4 color_attenuation;
	vec4 radius;
};

layout (std140, binding = 0) uniform LightBlock {
    Light lights[512];
} ;

layout (binding = 1) buffer supPixelLights{
    uint supPixelLight[];
} Data;

void main() {
	// get index of first affecting light in buffer of lights for this super-pixel
	uint idx = numSuperPixels_x * int(gl_FragCoord.y) * maxNumLights + int(gl_FragCoord.x) * maxNumLights;
	// get index where to put this light to buffer
	uint last = imageAtomicAdd(supPixelLightCounter, ivec2(gl_FragCoord.xy), 1);
	uint newLightIdx = idx + last;

	// put id of light
	Data.supPixelLight[newLightIdx] = int(lights[instance_ID].curr_pos_id.w);

	if (last > 0){
		// there are at least two lights affecting this super-pixel -> draw with white color
		FragColor = vec4(1.0);
	}
	else {
		FragColor = vec4(0.0, 0.0, 1.0, 1.0);
	}
}
