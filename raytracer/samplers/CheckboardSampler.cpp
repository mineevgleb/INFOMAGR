#include "CheckboardSampler.h"

namespace AGR {

	CheckboardSampler::CheckboardSampler(const glm::vec2& patternSize,
		const glm::vec3& color1, const glm::vec3& color2)
		: m_patternSize(patternSize), m_color1(color1), m_color2(color2)
	{}

	void CheckboardSampler::getColor(const glm::vec2 & texcoord, glm::vec3 & outColor) const
	{
		glm::vec2 normTexCoord(texcoord.x - glm::floor(texcoord.x), 
			texcoord.y - glm::floor(texcoord.y));
		normTexCoord *= m_patternSize;
		normTexCoord.x -= glm::floor(normTexCoord.x);
		normTexCoord.y -= glm::floor(normTexCoord.y);
		if ((normTexCoord.x > 0.5 && normTexCoord.y > 0.5) || 
			(normTexCoord.x < 0.5 && normTexCoord.y < 0.5)) {
			outColor = m_color1;
		}
		else {
			outColor = m_color2;
		}
	}

	void CheckboardSampler::setColors(const glm::vec3 & color1, const glm::vec3 & color2)
	{
		m_color1 = color1;
		m_color2 = color2;
	}

	void CheckboardSampler::getColors(glm::vec3 & color1, glm::vec3 & color2) const
	{
		color1 = m_color1;
		color2 = m_color2;
	}

	void CheckboardSampler::setPatternSize(const glm::vec2& size)
	{
		m_patternSize = size;
	}

	const glm::vec2& CheckboardSampler::getPatternSize() const
	{
		return m_patternSize;
	}

}
