#include "SceneObject.h"
#include <glm/gtc/matrix_transform.hpp>

namespace AGR {

	const Transform & SceneObject::getTransform() const
	{
		return m_transform;
	}

	const glm::mat4x4 & SceneObject::getTransformMat()
	{
		if (m_isModified) {
			m_transformMat = glm::mat4x4();
			m_transformMat = glm::translate(m_transformMat, m_transform.position);
			m_transformMat = glm::rotate(m_transformMat, m_transform.rotation.y, glm::vec3(0, 1, 0));
			m_transformMat = glm::rotate(m_transformMat, m_transform.rotation.x, glm::vec3(1, 0, 0));
			m_transformMat = glm::rotate(m_transformMat, m_transform.rotation.z, glm::vec3(0, 0, 1));
			m_transformMat = glm::scale(m_transformMat, m_transform.scale);
			m_isModified = false;
		}
		return m_transformMat;
	}

	void SceneObject::setTransform(const Transform & t)
	{
		m_transform = t;
		m_isModified = true;
	}

	void SceneObject::modifyTransform(const Transform & t)
	{
		m_transform.position += t.position;
		m_transform.rotation += t.rotation;
		m_transform.scale *= t.scale;
		m_isModified = true;
	}

}