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

	float Sphere::intersect(const Ray &r) const
	{
		glm::vec3 sphere2ray = r.origin - m_position;
		float a = glm::dot(r.direction, r.direction);
		float b = glm::dot(r.direction * 2.0f, sphere2ray);
		float c = glm::dot(sphere2ray, sphere2ray) - m_radius * m_radius;
		float dsqr = b * b - 4 * a * c;
		if (dsqr < 0) return false;
		if (dsqr < FLT_EPSILON) {
			float t = -b / (2 * a);
			if (t > 0) {
				return t;
			}
		}
		float d = sqrt(dsqr);
		float t = (-b - d) / (2 * a);
		if (t < 0) {
			t = (-b + d) / (2 * a);
		}
		return t;
	}

	void Sphere::getTexCoordAndNormal(const Ray& r, float dist,
		glm::vec2& texCoord, glm::vec3& normal) const
	{
		glm::vec3 hitPt = r.direction * dist + r.origin;
		if (m_material->isTexCoordRequired()) {
			glm::vec4 rotatedCoord = glm::vec4((hitPt - m_position) / m_radius, 1);
			rotatedCoord = m_rotationMatrix * rotatedCoord;
			texCoord.x = 0.5f + glm::atan(-rotatedCoord.z, -rotatedCoord.x) / (2 * M_PI);
			texCoord.y = 0.5f - glm::asin(-rotatedCoord.y) / M_PI;
		}
		normal = (hitPt - m_position) / m_radius;
		if (glm::dot(-r.direction, normal) < 0) {
			normal *= -1;
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
