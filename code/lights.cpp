#include "lights.h"

void generate_lights(int N, std::vector<Light> & out_lights, float pos_from, float pos_to, float max_radius) {

	if (out_lights.size() != N) {
		out_lights.resize(N);
	}

	Light light;
	for (int i = 0; i < N; i++) {
		// position and id
		light.curr_pos_id = glm::vec4(glm::compRand3(pos_from, pos_to), i);
		// light radius
		float radius = glm::compRand1(1.0f, max_radius);
		light.radius = glm::vec4(radius);
		// attenuation factor
		float b = 1.0 / (radius*radius * 0.1f);
		// color and attenuation
		light.color_attenuation = glm::vec4(glm::compRand3(0.0f, 1.0f), b);
		// original position and radius of rotation
		light.original_pos_radius = light.curr_pos_id;
		light.original_pos_radius.w = glm::compRand1(0.15f, 2.0f);
	
		out_lights[i] = light;
	}
}
