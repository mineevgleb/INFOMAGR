#pragma once
#include "structs.h"
namespace AGR {

	class SceneObject {
		friend class Scene;
	public:
		SceneObject(const Transform &transform) : m_transform(transform), m_idx(-1), m_isModified(true) {}
		const Transform& getTransform() const;

		//Set object's transrom to given values
		virtual void setTransform(const Transform& t);

		//Modify object's transfrom by given relative values
		virtual void modifyTransform(const Transform& t);
	protected:
		const glm::mat4x4& getTransformMat();

		Transform m_transform;
	private:
		//index of an object in the scene's vector
		int m_idx;
		bool m_isModified;
		glm::mat4x4 m_transformMat;
	};
}