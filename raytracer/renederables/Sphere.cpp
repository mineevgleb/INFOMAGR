#include "Sphere.h"

namespace AGR {

	void Sphere::intersect(const Ray & r, std::vector<Intersection> & out)
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

	void Sphere::getTexCoord(glm::vec3 pt, glm::vec2& out)
	{
		m_transform.scale.y = m_transform.scale.z = m_transform.scale.x;
		pt -= m_transform.position;
		pt /= m_transform.scale.x;
		glm::vec4 rotatedCoord = glm::vec4(pt.x, pt.y, pt.z, 1);
		rotatedCoord = getTransformMat() * rotatedCoord;
		pt.x = rotatedCoord.x;
		pt.y = rotatedCoord.y;
		pt.z = rotatedCoord.z;
		pt -= m_transform.position;
		pt = pt / m_transform.scale.x;
		const float M_PI = 3.141592653589f;
		out.x = 0.5f + glm::atan(-pt.z, -pt.x) / (2 * M_PI);
		out.y = 0.5f - glm::asin(-pt.y) / M_PI;
	}

}