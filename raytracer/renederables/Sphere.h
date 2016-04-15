#pragma once
#include "Primitive.h"

namespace AGR {

	class Sphere : public Primitive {
	public:
		Sphere(Material& m, const glm::vec3& position = glm::vec3(), float radius = 1.0f,
			const glm::vec3& rotation = glm::vec3());

		float intersect(const Ray &r) const override;
		void getTexCoordAndNormal(const Ray& r, float dist,
			glm::vec2& texCoord, glm::vec3& normal) const override;

		const glm::vec3& getPosition() const;
		void setPosition(const glm::vec3& position);

		const glm::vec3& getRotation() const;
		void setRotation(const glm::vec3& rotation);

		float getRadius() const;
		void setRadius(float radius);

		void commitTransformations();
		glm::vec3 getRandomPoint() override;
		float getArea() override;
		float calcSolidAngle(glm::vec3& pt) override;
	private:
		bool m_modified;
		glm::vec3 m_position;
		glm::vec3 m_rotation;
		float m_radius;
		glm::mat4x4 m_rotationMatrix;
	};

}