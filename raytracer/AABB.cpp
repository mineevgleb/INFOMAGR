#include "AABB.h"
#include "cmath"

namespace AGR
{
	AABB::AABB(glm::vec3& minPt, glm::vec3& maxPt)
		:m_minPt(minPt), m_maxPt(maxPt)
	{}

	AABB::AABB(const std::vector<AABB>& boxes)
		: AABB(boxes[0])
	{
		for (int i = 1; i < boxes.size(); ++i) {
			extend(boxes[i]);
		}
	}

	bool AABB::testIntersection(const Ray& r, const glm::vec3& invDir) const
	{
		glm::vec3 t1 = (m_minPt - r.origin) * invDir;
		glm::vec3 t2 = (m_maxPt - r.origin) * invDir;
	
		float tmin = fmax(fmax(fmin(t1.x, t2.x), fmin(t1.y, t2.y)), fmin(t1.z, t2.z));
		float tmax = fmin(fmin(fmax(t1.x, t2.x), fmax(t1.y, t2.y)), fmax(t1.z, t2.z));

		return (tmax > 0 && tmax > tmin);
	}

	void AABB::extend(const AABB& other)
	{
		m_minPt.x = fmin(other.m_minPt.x, m_minPt.x);
		m_minPt.y = fmin(other.m_minPt.y, m_minPt.y);
		m_minPt.z = fmin(other.m_minPt.z, m_minPt.z);
		m_maxPt.x = fmax(other.m_maxPt.x, m_maxPt.x);
		m_maxPt.y = fmax(other.m_maxPt.y, m_maxPt.y);
		m_maxPt.z = fmax(other.m_maxPt.z, m_maxPt.z);
	}

	glm::vec3 AABB::getCenter() const
	{
		return (m_minPt + m_maxPt) * 0.5f;
	}

	glm::vec3 AABB::getDimensions() const
	{
		return (m_maxPt - m_minPt) * 0.5f;
	}

	const glm::vec3& AABB::getMinPt() const
	{
		return m_minPt;
	}

	const glm::vec3& AABB::getMaxPt() const
	{
		return m_minPt;
	}
}
