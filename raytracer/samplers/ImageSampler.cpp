#include "ImageSampler.h"

namespace AGR {

	ImageSampler::ImageSampler(const std::string& filename, float HDRintensity, float HDRcontrast)
	{
		LoadImage(filename, HDRintensity, HDRcontrast);
	}

	ImageSampler::~ImageSampler()
	{
		FreeImage_Unload(m_bmp);
		if (isHDR) {
			FreeImage_Unload(m_HDRbmp);
		}
	}

	void ImageSampler::getColor(const glm::vec2& texcoord, glm::vec3& outColor) const
	{

		RGBTRIPLE col;
		unsigned x = static_cast<unsigned>((texcoord.x - glm::floor(texcoord.x)) * m_resolution.x);
		unsigned y = static_cast<unsigned>((texcoord.y - glm::floor(texcoord.y)) * m_resolution.y);
		col = (reinterpret_cast<RGBTRIPLE *>(FreeImage_GetScanLine(m_bmp, y)))[x];
		//FreeImage_GetPixelColor(m_bmp,
		//	static_cast<unsigned>((texcoord.x - glm::floor(texcoord.x)) * m_resolution.x),
		//	static_cast<unsigned>((texcoord.y - glm::floor(texcoord.y)) * m_resolution.y),
		//	&col);
		outColor.r = col.rgbtRed / 255.0f;
		outColor.g = col.rgbtGreen / 255.0f;
		outColor.b = col.rgbtBlue / 255.0f;
	}

	void ImageSampler::getHDRColor(const glm::vec2& texcoord, glm::vec3& outColor) const
	{
		if (!isHDR) {
			getColor(texcoord, outColor);
		} else {
			FIRGBF col;
			unsigned x = static_cast<unsigned>((texcoord.x - glm::floor(texcoord.x)) * m_resolution.x);
			unsigned y = static_cast<unsigned>((texcoord.y - glm::floor(texcoord.y)) * m_resolution.y);
			col = (reinterpret_cast<FIRGBF *>(FreeImage_GetScanLine(m_HDRbmp, y)))[x];
			outColor.r = col.red;
			outColor.g = col.green;
			outColor.b = col.blue;
		}
	}

	void ImageSampler::LoadImage(const std::string& filename, float HDRintensity, float HDRcontrast)
	{
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.c_str(), 0);
		if (fif == FIF_UNKNOWN) fif = FreeImage_GetFIFFromFilename(filename.c_str());
		isHDR = FIF_EXR == fif || FIF_HDR == fif || FIF_TIFF == fif;
		if (isHDR) {
			m_HDRbmp = FreeImage_Load(fif, filename.c_str());
			m_bmp = FreeImage_TmoReinhard05(m_HDRbmp, HDRintensity, HDRcontrast);
			m_resolution.x = FreeImage_GetWidth(m_bmp);
			m_resolution.y = FreeImage_GetHeight(m_bmp);
		}
		else {
			m_bmp = FreeImage_Load(fif, filename.c_str());
			m_resolution.x = FreeImage_GetWidth(m_bmp);
			m_resolution.y = FreeImage_GetHeight(m_bmp);
		}
	}
}
