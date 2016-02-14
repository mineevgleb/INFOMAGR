#pragma once
#include "SceneObject.h"
#include <vector>

namespace AGR {

	class Renderable : public SceneObject {
	public:
		Renderable(const Transform &transform) : SceneObject(transform) {}
		virtual void intersect(const Ray &r, std::vector<Intersection> &out) const = 0;
	};

}