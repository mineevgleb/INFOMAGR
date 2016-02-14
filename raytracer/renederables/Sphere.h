#pragma once
#include "../Renderable.h"

namespace AGR {

	class Sphere : public Renderable {
	public:
		//X component of the scale is used as the sphere's radius
		Sphere(const Transform &transform) : Renderable(transform) {}

		virtual void intersect(const Ray &r, std::vector<Intersection> &out) const;
	};

}