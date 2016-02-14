#pragma once
#include <glm\glm.hpp>

namespace AGR {

	class Sampler {
	public:
		virtual void getColor(const glm::vec2& texcoord, glm::vec3 &outColor) const = 0;
	};

}