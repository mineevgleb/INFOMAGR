#include "PointLight.h"
#include "../util.h"

namespace AGR
{
	PointLight::PointLight(float constIntensityRadius, 
		const glm::vec3& position, 
		const glm::vec3& color, 
		float angle, 
		const glm::vec3& direction)
		: m_constIntensityRadius(constIntensityRadius),
	      m_position(position),
	      m_color(color),
	      m_angle(angle),
	      m_direction(direction) {}

	void PointLight::getDirectionToTheLight(const glm::vec3& pt, LightRay& dir) const
	{
		dir.length = glm::length(pt - m_position);
		dir.direction = (m_position - pt) / dir.length;
	}

	void PointLight::getIntensityAtThePoint(const glm::vec3& pt, glm::vec3& col) const
	{
		if (m_angle < 180.0f)
		{
			glm::vec3 vecToPt = glm::normalize(pt - m_position);
			float curAngle = glm::acos(glm::dot(vecToPt, m_direction));
			curAngle *= 180.0f / M_PI;
			if (curAngle > m_angle)
			{
				col = glm::vec3();
				return;
			}
		}
		float r = glm::length(pt - m_position);
		if (r < m_constIntensityRadius)
		{
			col = m_color;
			return;
		}
		col = m_color *
			(m_constIntensityRadius * m_constIntensityRadius) / (r * r);
	}

	float PointLight::getConstIntensityRadius() const
	{
		return m_constIntensityRadius;
	}

	void PointLight::setConstIntensityRadius(float constIntensityRadius)
	{
		m_constIntensityRadius = constIntensityRadius;
	}

	const glm::vec3& PointLight::getPosition() const
	{
		return m_position;
	}

	void PointLight::setPosition(const glm::vec3& position)
	{
		m_position = position;
	}

	const glm::vec3& PointLight::getColor() const
	{
		return m_color;
	}

	void PointLight::setColor(const glm::vec3& color)
	{
		m_color = color;
	}

	const float& PointLight::getAngle() const
	{
		return m_angle;
	}

	void PointLight::setAngle(float angle)
	{
		m_angle = angle;
	}

	const glm::vec3& PointLight::getDirection() const
	{
		return m_direction;
	}

	void PointLight::setDirection(const glm::vec3& direction)
	{
		m_direction = direction;
	}
}
