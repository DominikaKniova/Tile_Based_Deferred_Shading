#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "./glm/glm.hpp"
#include "./glm/gtx/random.hpp"
#include <vector>
#include <random>

struct Light {
	// aligned to 16 bytes per element
	glm::vec4 curr_pos_id;				 // current position of light and light id	
	glm::vec4 original_pos_radius;       // original position of light and radius of rotation
	glm::vec4 color_attenuation;		 // color of the light and its attenuation		
	glm::vec4 radius;					 // light radius
};

void generate_lights(int N, std::vector<Light> & out_lights, float pos_from, float pos_to, float max_radius);

#endif

