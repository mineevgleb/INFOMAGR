#include "Sphere.h"

namespace AGR {
	Sphere::Sphere(Material& m, const glm::vec3& position,
		float radius, const glm::vec3& rotation) : Renderable(m),
		m_position(position),
		m_rotation(rotation),
		m_radius(radius),
		m_rotationMatrix()
	{
		rotateMat(m_rotation, m_rotationMatrix);
	}

	void Sphere::intersect(const Ray & r, std::vector<Intersection> & out) const
	{
		glm::vec3 sphere2ray = r.origin - m_position;
		float a = glm::dot(r.directon, r.directon);
		float b = glm::dot(r.directon * 2.0f, sphere2ray);
		float c = glm::dot(sphere2ray, sphere2ray) - m_radius * m_radius;
		float dsqr = b * b - 4 * a * c;
		if (dsqr < 0) return;
		if (abs(dsqr) < FLT_EPSILON) {
			float t = -b / 2 * a;
			glm::vec3 intersectPt = r.directon * t + r.origin;
			glm::vec3 normal = (intersectPt - m_position) / m_position;
			out.push_back(Intersection(t, normal, this));
			return;
		}
		float d = sqrt(dsqr);
		float t = (-b + d) / 2 * a;
		glm::vec3 intersectPt = r.directon * t + r.origin;
		glm::vec3 normal = (intersectPt - m_position) / m_position;
		out.push_back(Intersection(t, normal, this));
		t = (-b - d) / 2 * a;
		intersectPt = r.directon * t + r.origin;
		normal = (intersectPt - m_position) / m_position;
		out.push_back(Intersection(t, normal, this));
	}

	void Sphere::getTexCoord(glm::vec3 pt, glm::vec2& out) const
	{
		pt -= m_position;
		pt /= m_radius;
		glm::vec4 rotatedCoord = glm::vec4(pt.x, pt.y, pt.z, 1);
		rotatedCoord = m_rotationMatrix * rotatedCoord;
		pt = glm::vec3(rotatedCoord);
		out.x = 0.5f + glm::atan(-pt.z, -pt.x) / (2 * M_PI);
		out.y = 0.5f - glm::asin(-pt.y) / M_PI;
	}

	const glm::vec3& Sphere::getPosition() const
	{
		return m_position;
	}

	void Sphere::setPosition(const glm::vec3& position)
	{
		m_position = position;
	}

	const glm::vec3& Sphere::getRotation() const
	{
		return m_rotation;
	}

	void Sphere::setRotation(const glm::vec3& rotation)
	{
		m_rotation = rotation;
		m_rotationMatrix = glm::mat4x4();
		rotateMat(rotation, m_rotationMatrix);
	}

	float Sphere::getRadius() const
	{
		return m_radius;
	}

	void Sphere::setRadius(float radius)
	{
		m_radius = radius;
	}
}
