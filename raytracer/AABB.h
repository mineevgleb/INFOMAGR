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
		AABB(const glm::vec3& minPt, const glm::vec3& maxPt);
		AABB(const std::vector<AABB>& boxes);
		bool intersect(const Ray &r, float& dist, glm::vec3& invDir) const;
		bool testOverlap(const AABB& other) const;
		void extend(const AABB& other);
		glm::vec3 getCenter() const;
		glm::vec3 getDimensions() const;
		const glm::vec3& getMinPt() const;
		const glm::vec3& getMaxPt() const;
		float calcArea() const;
	private:
		glm::vec3 m_minPt;
		glm::vec3 m_maxPt;
	};
}