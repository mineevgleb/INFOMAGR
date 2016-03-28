#pragma once
#include "Primitive.h" 
#include "Triangle.h"
#include <vector>

namespace AGR {
	enum NormalType
	{
		FLAT, SMOOTH, CONSISTENT
	};

	class Mesh
	{
		friend class Renderer;
	public:
		Mesh(Material& m) 
			: m_material(&m),
			m_translation(0),
			m_rotation(0),
			m_scale(1){}
		~Mesh() { release(); }
		bool load(const std::string& path, NormalType nt = CONSISTENT);
		void setPosition(const glm::vec3& p);
		void setRotation(const glm::vec3& r);
		void setScale(const glm::vec3& s);
		void commitTransformations();
		void release();
	private:
		std::vector<Triangle *> m_triangles;
		Material *m_material;

		glm::mat4x4 m_modMatrix;
		glm::mat4x4 m_normModMatrix;
	    glm::vec3 m_translation;
		glm::vec3 m_rotation;
		glm::vec3 m_scale;
	};
}