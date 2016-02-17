#pragma once
#include "Light.h"

namespace AGR {
	class GlobalLight : public Light
	{
	public:
		GlobalLight (const glm::vec3& direction = glm::vec3(1, 1, 1), 
			const glm::vec3& intensity = glm::vec3(0, -1, 0)) 
			: m_direction(direction),
			m_intensity(intensity)
		{}
		virtual void getDirectionToTheLight(const glm::vec3 &pt, glm::vec3& dir) const
		{
			dir = -m_direction;
		};
		virtual void getIntensityAtThePoint(const glm::vec3 &pt, glm::vec3& col) const
		{
			col = m_intensity;
		};

		const glm::vec3& getDirection() const
		{
			return m_direction;
		}

		void setDirection(const glm::vec3& direction)
		{
			m_direction = direction;
		}

		const glm::vec3& getIntensity() const
		{
			return m_intensity;
		}

		void setIntensity(const glm::vec3& intensity)
		{
			this->m_intensity = intensity;
		}

	private:
		glm::vec3 m_direction;
		glm::vec3 m_intensity;
	};
}