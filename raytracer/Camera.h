#pragma once
#include "SceneObject.h"

namespace AGR {

	class Camera : SceneObject {
	public:
		//For camera scale values X and Y are used to determine height and width of the screen.
		//Z scale determines distance drom camera origin to the screen.
		//With rotation 0, 0, 0 camera looks to the positiove direction of Z axis.
		Camera(const Transform &transform) : SceneObject(transform) { updateVectors(); }

		virtual void setTransform(const Transform& t) { m_isModified = true; SceneObject::setTransform(t); }
		virtual void modifyTransform(const Transform& t) { m_isModified = true; SceneObject::modifyTransform(t); }

		//Ray through the pixel with coordinates X and Y. Coordinates are in range [0, 1].
		void produceRay(const glm::vec2 position, Ray &out);
	private:
		void updateVectors();

		glm::vec3 m_frontVec;
		glm::vec3 m_rightVec;
		glm::vec3 m_upVec;

		bool m_isModified;
	};

}
