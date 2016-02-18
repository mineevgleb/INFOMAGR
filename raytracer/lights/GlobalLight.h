#pragma once
#include "Light.h"

namespace AGR {
	class GlobalLight : public Light
	{
	public:
		GlobalLight(const glm::vec3& direction = glm::vec3(0, -1, 0),
		            const glm::vec3& intensity = glm::vec3(1, 1, 1));

		void getDirectionToTheLight(const glm::vec3& pt, LightRay& dir) const override;
		void getIntensityAtThePoint(const glm::vec3& pt, glm::vec3& col) const override;

		const glm::vec3& getDirection() const;
		void setDirection(const glm::vec3& direction);

		const glm::vec3& getIntensity() const;
		void setIntensity(const glm::vec3& intensity);

	private:
		glm::vec3 m_direction;
		glm::vec3 m_intensity;
	};
}