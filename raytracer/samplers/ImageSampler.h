#pragma once
#include "Sampler.h"
#include "FreeImage.h"
#include <string>

namespace AGR {

	class ImageSampler : public Sampler {
	public:
		ImageSampler(const std::string& filename, float HDRintensity = 0, float HDRcontrast = 0);
		~ImageSampler();
		void getColor(const glm::vec2& texcoord, glm::vec3 &outColor) const override;
		void getHDRColor(const glm::vec2& texcoord, glm::vec3 &outColor) const override;
		void LoadImage(const std::string& filename, float HDRintensity = 0, float HDRcontrast = 0);
	private:
		FIBITMAP *m_bmp;
		FIBITMAP *m_HDRbmp;
		glm::vec2 m_resolution;
		bool isHDR;
	};

	
}
