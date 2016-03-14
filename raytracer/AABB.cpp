#include "AABB.h"
#include "cmath"

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

	bool AABB::testIntersection(const Ray& r, const glm::vec3& invDir) const
	{
		glm::vec3 t1 = (m_minPt - r.origin) * invDir;
		glm::vec3 t2 = (m_maxPt - r.origin) * invDir;

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
		return m_maxPt;
	}

	float AABB::calcArea() const
	{
		glm::vec3 dim = m_maxPt - m_minPt;
		return (dim.x * (dim.y + dim.z) + dim.y * dim.z) * 2;
	}
}
