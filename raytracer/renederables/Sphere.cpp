#include "Sphere.h"
#include "../util.h"

namespace AGR {
	Sphere::Sphere(Material& m, const glm::vec3& position,
		float radius, const glm::vec3& rotation)
		: Primitive(m),
		m_modified(false),
		m_position(position),
		m_rotation(rotation),
		m_radius(radius),
		m_rotationMatrix()
	{
		rotateMat(m_rotation, m_rotationMatrix);
		commitTransformations();
	}

	bool Sphere::intersect(Intersection& intersect) const
	{
		Ray& r = intersect.ray;
		glm::vec3 sphere2ray = r.origin - m_position;
		float a = glm::dot(r.direction, r.direction);
		float b = glm::dot(r.direction * 2.0f, sphere2ray);
		float c = glm::dot(sphere2ray, sphere2ray) - m_radius * m_radius;
		float dsqr = b * b - 4 * a * c;
		if (dsqr < 0) return false;
		if (dsqr < FLT_EPSILON) {
			float t = -b / (2 * a);
			if (t > 0) {
				intersect.hitPt = r.direction * t + r.origin;
				intersect.ray_length = t;
				intersect.p_object = this;
				return true;
			}
		}
		float d = sqrt(dsqr);
		float t = (-b - d) / (2 * a);
		if (t < 0) {
			t = (-b + d) / (2 * a);
		}
		if (t < 0) {
			return false;
		}
		glm::vec3 intersectPt;
		glm::vec3 normal;
		intersect.hitPt = r.direction * t + r.origin;
		intersect.ray_length = t;
		intersect.p_object = this;
		return true;
	}

	void Sphere::getTexCoordAndNormal(Intersection& intersect) const
	{
		if (m_material->isTexCoordRequired()) {
			glm::vec3 hitPt = intersect.hitPt;
			hitPt -= m_position;
			hitPt /= m_radius;
			glm::vec4 rotatedCoord = glm::vec4(hitPt, 1);
			rotatedCoord = m_rotationMatrix * rotatedCoord;
			intersect.texCoord.x = 0.5f + glm::atan(-rotatedCoord.z, -rotatedCoord.x) / (2 * M_PI);
			intersect.texCoord.y = 0.5f - glm::asin(-rotatedCoord.y) / M_PI;
		}
		intersect.normal = (intersect.hitPt - m_position) / m_radius;
		if (glm::dot(-intersect.ray.direction, intersect.normal) < 0) {
			intersect.normal *= -1;
		}
	}

	const glm::vec3& Sphere::getPosition() const
	{
		return m_position;
	}

	void Sphere::setPosition(const glm::vec3& position)
	{
		m_position = position;
		m_modified = true;
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
		m_modified = true;
	}

	void Sphere::commitTransformations()
	{
		glm::vec3 minPt = m_position - glm::vec3(m_radius);
		glm::vec3 maxPt = m_position + glm::vec3(m_radius);
		m_aabb = AABB(minPt, maxPt);
	}
}
