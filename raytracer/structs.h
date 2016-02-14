#pragma once
#include "glm\glm.hpp"
#include "samplers\Sampler.h"

namespace AGR {

	struct Ray {
		glm::vec3 origin;
		glm::vec3 directon;
	};

	class Renderable;
	struct Intersection {
		Intersection(float ray_length = FLT_MAX, 
			const glm::vec3& normal = glm::vec3(0, 0, 1), 
			Renderable *p_object = nullptr,
			const glm::vec2& texCoord = glm::vec2())
			: ray_length(ray_length), 
			normal(normal), 
			p_object(p_object),
			texCoord(texCoord) {}
		float ray_length;
		glm::vec3 normal;
		Renderable *p_object;
		glm::vec2 texCoord;
	};

	struct Transform {
		Transform(const glm::vec3 &position = glm::vec3(0, 0, 0),
			const glm::vec3 &rotation = glm::vec3(0, 0, 0),
			const glm::vec3 &scale = glm::vec3(1, 1, 1))
			:position(position), rotation(rotation), scale(scale) {}
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
	};

	struct Material {
		bool isRequiresTexCoord;
		float ambientIntensity;
		Sampler* ambientColor;
		float diffuseIntensity;
		Sampler* diffuseColor;
	};
}
