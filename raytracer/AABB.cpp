#include "AABB.h"

namespace AGR
{
	AABB::AABB(const glm::vec3& minPt, const glm::vec3& maxPt)
		:m_minPt(minPt), m_maxPt(maxPt)
	{}

	AABB::AABB(const std::vector<AABB>& boxes)
		: AABB(boxes[0])
	{
		for (int i = 1; i < boxes.size(); ++i) {
			extend(boxes[i]);
		}
	}

	bool AABB::intersect(const Ray& r, float &dist) const
	{
		glm::vec3 t1 = (m_minPt - r.origin) * r.invDirection;
		glm::vec3 t2 = (m_maxPt - r.origin) * r.invDirection;
		bool cmp = t1.x < t2.x;
		float xmin = t2.x + (t1.x - t2.x) * cmp;
		float xmax = t1.x + t2.x - xmin;
		cmp = t1.y < t2.y;
		float ymin = t2.y + (t1.y - t2.y) * cmp;
		float ymax = t1.y + t2.y - ymin;
		cmp = t1.z < t2.z;
		float zmin = t2.z + (t1.z - t2.z) * cmp;
		float zmax = t1.z + t2.z - zmin;
		cmp = xmin > ymin;
		float tmin = ymin + (xmin - ymin) * cmp;
		cmp = zmin > tmin;
		tmin += cmp * (zmin - tmin);
		cmp = xmax < ymax;
		float tmax = ymax + (xmax - ymax) * cmp;
		cmp = zmax < tmax;
		tmax += cmp * (zmax - tmax);
		dist = tmin;
		return tmax > 0 && tmax > tmin;
	}

	bool AABB::testOverlap(const AABB& other) const 
	{
		glm::vec3 minPt = glm::max(m_minPt, other.m_minPt);
		glm::vec3 maxPt = glm::min(m_maxPt, other.m_maxPt);
		return (minPt.x < maxPt.x &&  minPt.y < maxPt.y && minPt.z < maxPt.z);
	}

	void AABB::extend(const AABB& other)
	{
		m_minPt = glm::min(m_minPt, other.m_minPt);
		m_maxPt = glm::max(m_maxPt, other.m_maxPt);
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
		return m_maxPt;
	}

	float AABB::calcArea() const
	{
		glm::vec3 dim = m_maxPt - m_minPt;
		return (dim.x * (dim.y + dim.z) + dim.y * dim.z) * 2;
	}
}
