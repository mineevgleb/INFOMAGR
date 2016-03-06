#include "Camera.h"
#include "util.h"

namespace AGR {
	Camera::Camera(float aspectRatio, float horFOV, const glm::vec3& position, const glm::vec3& rotation):
		m_aspectRatio(aspectRatio),
		m_horFov(horFOV / 180 * M_PI),
		m_position(position),
		m_rotation(rotation)
	{ updateVectors(); }

	void Camera::produceRay(const glm::vec2 position, Ray & out) const
	{
		out.origin = m_position;
		out.direction = glm::normalize(m_frontVec + 
			m_rightVec * (position.x - 0.5f) - m_upVec * (position.y - 0.5f));
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

	void Camera::updateVectors()
	{
		glm::vec4 front(0, 0, 1, 1);
		glm::vec4 right(0.5f, 0, 0, 1);
		glm::vec4 up(0, 0.5f, 0, 1);
		float camLen = (m_aspectRatio / 2) * glm::tan(m_horFov / 2);
		glm::vec3 scale(m_aspectRatio, 1.0f, camLen);
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
