#include "Camera.h"

namespace AGR {

	void Camera::produceRay(const glm::vec2 position, Ray & out)
	{
		if (m_isModified) updateVectors();
		out.origin = m_transform.position;
		out.directon = glm::normalize(m_frontVec + 
			m_rightVec * (position.x - 0.5f) - m_upVec * (position.y - 0.5f));
	}

	void Camera::updateVectors()
	{
		glm::vec4 front(0, 0, 1, 1);
		glm::vec4 right(0.5f, 0, 0, 1);
		glm::vec4 top(0, 0.5f, 0, 1);
		front = getTransformMat() * front;
		right = getTransformMat() * right;
		top = getTransformMat() * top;
		m_frontVec.x = front.x; m_frontVec.y = front.y; m_frontVec.z = front.z;
		m_rightVec.x = right.x; m_rightVec.y = right.y; m_rightVec.z = right.z;
		m_upVec.x = top.x; m_upVec.y = top.y; m_upVec.z = top.z;
		m_frontVec -= m_transform.position;
		m_rightVec -= m_transform.position;
		m_upVec -= m_transform.position;
		m_isModified = false;
	}

}
