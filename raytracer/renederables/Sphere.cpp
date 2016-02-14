#include "Sphere.h"

namespace AGR {

	void Sphere::intersect(const Ray & r, std::vector<Intersection> & out) const
	{
		glm::vec3 sphere2ray = r.origin - m_transform.position;
		float a = glm::dot(r.directon, r.directon);
		float b = glm::dot(r.directon * 2.0f, sphere2ray);
		float c = glm::dot(sphere2ray, sphere2ray) - m_transform.scale.x * m_transform.scale.x;
		float dsqr = b * b - 4 * a * c;
		if (dsqr < 0) return;
		if (abs(dsqr) < FLT_EPSILON) {
			float t = -b / 2 * a;
			glm::vec3 intersectPt = r.directon * t + r.origin;
			glm::vec3 normal = (intersectPt - m_transform.position) / m_transform.scale.x;
			out.push_back(Intersection(t, normal, this));
			return;
		}
		float d = sqrt(dsqr);
		float t = (-b + d) / 2 * a;
		glm::vec3 intersectPt = r.directon * t + r.origin;
		glm::vec3 normal = (intersectPt - m_transform.position) / m_transform.scale.x;
		out.push_back(Intersection(t, normal, this));
		t = (-b - d) / 2 * a;
		intersectPt = r.directon * t + r.origin;
		normal = (intersectPt - m_transform.position) / m_transform.scale.x;
		out.push_back(Intersection(t, normal, this));
	}

}