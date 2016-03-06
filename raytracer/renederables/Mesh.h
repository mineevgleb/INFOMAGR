#pragma once
#include "Renderable.h" 
#include "Triangle.h"
#include <vector>
#include <string>

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
			m_scale(1){}
		~Mesh() { release(); }
		bool load(const std::string& path, 
			NormalType nt = CONSISTENT, bool invertNormals = false);
		void setPosition(const glm::vec3& p, bool update = true);
		void setRotation(const glm::vec3& r, bool update = true);
		void setScale(const glm::vec3& s, bool update = true);
		void release();
	private:
		void updateTriangles();
		std::vector<Triangle *> m_triangles;
		Material *m_material;

		glm::mat4x4 m_modMatrix;
		glm::mat4x4 m_normModMatrix;
	    glm::vec3 m_translation;
		glm::vec3 m_rotation;
		glm::vec3 m_scale;
	};
}