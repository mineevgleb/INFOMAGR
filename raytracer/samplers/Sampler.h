#pragma once
#include <glm\glm.hpp>

namespace AGR {

	class Sampler {
	public:
		virtual void getColor(const glm::vec2& texcoord, glm::vec3 &outColor) const = 0;
		virtual void getHDRColor(const glm::vec2& texcoord, glm::vec3 &outColor) const
		{
			return getColor(texcoord, outColor);
		};
		virtual bool isRequiresTexture() const { return true; }
		virtual ~Sampler() {};
	};

}