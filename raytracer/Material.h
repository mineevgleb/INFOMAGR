#pragma once
#include "samplers\Sampler.h"

namespace AGR {
	struct Material {
		Material(){};
		bool isTexCoordRequired() const
		{
			return texture != nullptr && texture->isRequiresTexture();
		};
		union {
			float ambientIntensity = 0.0f;
			float glowIntensity;
		};
		float diffuseIntensity = 0.0f;
		union {
			float specularIntensity = 0.0f;
			float microfacetAlpha;
		};
		glm::vec3 innerColor; //For refractions
		float reflectionIntensity = 0.0f;
		float refractionIntensity = 0.0f;
		float refractionCoeficient = 0.0f;
		float absorption = 1.0f;
		float shininess = 0.0f;
		bool isMicrofacet = false;
		Sampler* texture = nullptr;
	};
}
