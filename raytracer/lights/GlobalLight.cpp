#include "GlobalLight.h"

namespace AGR
{
	GlobalLight::GlobalLight(const glm::vec3& direction, 
		const glm::vec3& intensity)
		: m_direction(glm::normalize(direction)),
		  m_intensity(intensity) {}

	void GlobalLight::getDirectionToTheLight(const glm::vec3& pt, LightRay& dir) const
	{
		dir.direction = -m_direction;
		dir.length = FLT_MAX;
	}

	void GlobalLight::getIntensityAtThePoint(const glm::vec3& pt, glm::vec3& col) const
	{
		col = m_intensity;
	}

	const glm::vec3& GlobalLight::getDirection() const
	{
		return m_direction;
	}

	void GlobalLight::setDirection(const glm::vec3& direction)
	{
		m_direction = glm::normalize(direction);
	}

	const glm::vec3& GlobalLight::getIntensity() const
	{
		return m_intensity;
	}

	void GlobalLight::setIntensity(const glm::vec3& intensity)
	{
		this->m_intensity = intensity;
	}
}
