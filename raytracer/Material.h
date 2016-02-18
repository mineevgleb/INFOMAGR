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
		float shininess = 0.0f;
		Sampler* texture = nullptr;
	};
}
