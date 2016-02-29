#pragma once
#include "Renderable.h"

namespace AGR {

	class Sphere : public Renderable {
	public:
		Sphere(Material& m, const glm::vec3& position = glm::vec3(), float radius = 1.0f,
			const glm::vec3& rotation = glm::vec3(), bool invertNormals = false);

		virtual bool intersect(const Ray &r, Intersection &out) const override;
		void getTexCoord(glm::vec3 pt, glm::vec2& out) const override;

		const glm::vec3& getPosition() const;
		void setPosition(const glm::vec3& position);

		const glm::vec3& getRotation() const;
		void setRotation(const glm::vec3& rotation);

		float getRadius() const;
		void setRadius(float radius);

	private:
		glm::vec3 m_position;
		glm::vec3 m_rotation;
		float m_radius;
		glm::mat4x4 m_rotationMatrix;
	};

}