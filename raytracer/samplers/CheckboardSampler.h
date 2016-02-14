#pragma once
#include "Sampler.h"

namespace AGR {

	class CheckboardSampler : public Sampler {
	public:
		CheckboardSampler(const glm::vec2& patternSize = glm::vec2(1),
			const glm::vec3& color1 = glm::vec3(), const glm::vec3& color2 = glm::vec3(1));
		virtual void getColor(const glm::vec2& texcoord, glm::vec3 &outColor) const;
		void setColors(const glm::vec3& color1, const glm::vec3& color2);
		void getColors(glm::vec3& color1, glm::vec3& color2) const;
		void setPatternSize(const glm::vec2& size);
		const glm::vec2& getPatternSize() const;
	private:
		glm::vec2 m_patternSize;
		glm::vec3 m_color1;
		glm::vec3 m_color2;
	};

}