#pragma once
#include "Sampler.h"

namespace AGR {

	class ColorSampler : public Sampler {
	public:
		ColorSampler(const glm::vec3& color = glm::vec3()) : m_color(color) {};
		virtual void getColor(const glm::vec2& texcoord, glm::vec3 &outColor) const {
			outColor = m_color;
		};
		void setColor(const glm::vec3& color) {
			m_color = color;
		}
	private:
		glm::vec3 m_color;
	};

}