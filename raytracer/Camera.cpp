#include "Camera.h"
#include "util.h"
#include <random>

namespace AGR {
	Camera::Camera(float aspectRatio, float horFOV, const glm::vec3& position, const glm::vec3& rotation):
		m_aspectRatio(aspectRatio),
		m_horFov(horFOV / 180 * M_PI),
		m_position(position),
		m_rotation(rotation)
	{ updateVectors(); }

	void Camera::produceRay(const glm::vec2 position, Ray & out) const
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<> distr;
		glm::vec2 offset = glm::vec2();
		if (m_apertureSize > FLT_EPSILON) {
			if (m_edgesAm < 3) {
				float r = glm::sqrt(distr(gen));
				float angle = distr(gen) * 2 * M_PI;
				offset = glm::vec2(r * glm::cos(angle), r * glm::sin(angle));
			} else {
				std::uniform_int_distribution<> distrInt(0, m_edgesAm - 1);
				int triangeNum = distrInt(gen);
				float angle1 = m_lensRotation + (M_PI * 2 / m_edgesAm) * triangeNum;
				float angle2 = m_lensRotation + (M_PI * 2 / m_edgesAm) * (triangeNum + 1);
				glm::vec2 side1 = glm::vec2(cos(angle1), sin(angle1));
				glm::vec2 side2 = glm::vec2(cos(angle2), sin(angle2));
				glm::vec2 coef = glm::vec2(distr(gen), distr(gen));
				if (coef.x + coef.y > 1.0f) coef = 1.0f - coef;
				offset = side1 * coef.x + side2 * coef.y;
			}
			offset = offset * m_apertureSize;
		}
		glm::vec3 globalOffset = offset.x * m_rightVec * 2.0f + offset.y * m_upVec * 2.0f;
		out.origin = m_position + globalOffset;
		out.direction = glm::normalize(m_frontVec * m_focalDist +
			m_rightVec * (position.x - 0.5f) * m_focalDist
			- m_upVec * (position.y - 0.5f) * m_focalDist - 
			globalOffset);
			
	}

	void Camera::lookAt(glm::vec3 point, float rollAngle)
	{
		lookAtToAngles(point - m_position, m_rotation);
		m_rotation.z = rollAngle;
		updateVectors();
	}

	float Camera::getAspectRatio() const
	{
		return m_aspectRatio;
	}

	void Camera::setAspectRatio(float aspect_ratio)
	{
		m_aspectRatio = aspect_ratio;
		updateVectors();
	}

	float Camera::getHorFOV() const
	{
		return m_horFov;
	}

	void Camera::setHorFOV(float horFOV)
	{
		m_horFov = horFOV / 180 * M_PI;
		updateVectors();
	}

	const glm::vec3& Camera::getPosition() const
	{
		return m_position;
	}

	void Camera::setPosition(const glm::vec3& position)
	{
		m_position = position;
		updateVectors();
	}

	const glm::vec3& Camera::getRotation() const
	{
		return m_rotation;
	}

	void Camera::setRotation(const glm::vec3& rotation)
	{
		m_rotation = rotation;
		updateVectors();
	}

	void Camera::setLensParams(float focalDist, float apertureSize, 
		int edgesAm, float roatation)
	{
		m_focalDist = focalDist;
		m_apertureSize = apertureSize;
		m_edgesAm = edgesAm;
		m_lensRotation = roatation;
	}

	void Camera::updateVectors()
	{
		glm::vec4 front(0, 0, 1, 1);
		glm::vec4 right(0.5f, 0, 0, 1);
		glm::vec4 up(0, 0.5f, 0, 1);
		float scaler = glm::tan(m_horFov / 2);
		glm::vec3 scale(m_aspectRatio * scaler, scaler, 1.0f);
		glm::mat4x4 transform;
		transformMat(glm::vec3(), m_rotation, scale, transform);
		front = transform * front;
		right = transform * right;
		up = transform * up;
		m_frontVec = glm::vec3(front);
		m_rightVec = glm::vec3(right);
		m_upVec = glm::vec3(up);
	}

}
