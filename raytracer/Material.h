#pragma once
#include "samplers\Sampler.h"

namespace AGR {
	struct Material {
		bool isTexCoordRequired() const
		{
			return texture != nullptr && texture->isRequiresTexture();
		};
		float ambientIntensity = 0.0f;
		float diffuseIntensity = 0.0f;
		float specularIntensity = 0.0f;
		float reflectionIntensity = 0.0f;
		float refractionIntensity = 0.0f;
		float refractionCoeficient = 0.0f;
		float absorption = 1.0f;
		float shininess = 0.0f;
		Sampler* texture = nullptr;
		glm::vec3 innerColor; //For refractions
	};
}
