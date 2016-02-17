#include "ImageSampler.h"

namespace AGR {

	ImageSampler::ImageSampler(const std::string& filename)
	{
		LoadImage(filename);
	}

	ImageSampler::~ImageSampler()
	{
		FreeImage_Unload(m_bmp);
	}

	void ImageSampler::getColor(const glm::vec2& texcoord, glm::vec3& outColor) const
	{
		RGBQUAD col;
		FreeImage_GetPixelColor(m_bmp,
			static_cast<unsigned>(texcoord.x * m_resolution.x),
			static_cast<unsigned>(texcoord.y * m_resolution.y),
			&col);
		outColor.r = col.rgbRed / 255.0f;
		outColor.g = col.rgbGreen / 255.0f;
		outColor.b = col.rgbBlue / 255.0f;
	}

	void ImageSampler::LoadImage(const std::string& filename)
	{
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(filename.c_str(), 0);
		if (fif == FIF_UNKNOWN) fif = FreeImage_GetFIFFromFilename(filename.c_str());
		m_bmp = FreeImage_Load(fif, filename.c_str());
		m_resolution.x = FreeImage_GetWidth(m_bmp);
		m_resolution.y = FreeImage_GetHeight(m_bmp);
	}
}
