#pragma once
#include "Renderable.h"

namespace AGR {

	class Sphere : public Renderable {
	public:
		//X component of the scale is used as the sphere's radius
		Sphere(Material& m, const Transform& transform) : Renderable(transform, m) {}

		Sphere(Material& m, const glm::vec3& position = glm::vec3(), float radius = 1.0f, const glm::vec3& rotation = glm::vec3())
			: Sphere(m, Transform(position, rotation, glm::vec3(radius))) {}

		virtual void intersect(const Ray &r, std::vector<Intersection> &out);
		void getTexCoord(glm::vec3 pt, glm::vec2& out);
	};

}