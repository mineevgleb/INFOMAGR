#pragma once
#include "Sampler.h"
#include "FreeImage.h"
#include <string>

namespace AGR {

	class ImageSampler : public Sampler {
	public:
		ImageSampler(const std::string& filename);
		~ImageSampler();
		virtual void getColor(const glm::vec2& texcoord, glm::vec3 &outColor) const override;
		void LoadImage(const std::string& filename);
	private:
		FIBITMAP *m_bmp;
		glm::vec2 m_resolution;
	};

	
}
