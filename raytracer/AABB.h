#pragma once

#include <glm/glm.hpp>
#include "Intersection.h"
#include <vector>

namespace AGR
{
	class AABB
	{
	public:
		AABB(){}
		AABB(glm::vec3& minPt, glm::vec3& maxPt);
		AABB(const std::vector<AABB>& boxes);
		bool testIntersection(const Ray &r, const glm::vec3& invDir) const;
		void extend(const AABB& other);
		glm::vec3 getCenter() const;
		glm::vec3 getDimensions() const;
		const glm::vec3& getMinPt() const;
		const glm::vec3& getMaxPt() const;
	private:
		glm::vec3 m_minPt;
		glm::vec3 m_maxPt;
	};
}