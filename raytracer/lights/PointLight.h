#pragma once
#include "Light.h"

namespace AGR{

class PointLight : public Light
{
public:
	PointLight(float constIntensityRadius,
	           const glm::vec3& position = glm::vec3(),
	           const glm::vec3& color = glm::vec3(1, 1, 1),
	           float angle = 190.0f,
	           const glm::vec3& direction = glm::vec3(0, -1, 0));
	void getDirectionToTheLight(const glm::vec3& pt, LightRay& dir) const override;
	void getIntensityAtThePoint(const glm::vec3& pt, glm::vec3& col) const override;


	float getConstIntensityRadius() const;
	void setConstIntensityRadius(float constIntensityRadius);

	const glm::vec3& getPosition() const;
	void setPosition(const glm::vec3& position);

	const glm::vec3& getColor() const;
	void setColor(const glm::vec3& color);

	const float& getAngle() const;
	void setAngle(float angle);

	const glm::vec3& getDirection() const;
	void setDirection(const glm::vec3& direction);

private:
	float m_constIntensityRadius;
	glm::vec3 m_position;
	glm::vec3 m_color;
	float m_angle;
	glm::vec3 m_direction;
};

}