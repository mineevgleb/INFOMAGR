#pragma once
#include "Renderable.h" 
#include "Triangle.h"
#include <vector>
#include <string>

namespace AGR {
	class MeshTriangle;
	class Mesh
	{
		friend class Renderer;
	public:
		Mesh(Material& m, bool invertNormals) 
			: m_material(&m),
			m_invertNormals(invertNormals){}
		~Mesh() { release(); }
		bool load(const std::string &path);
		void release();
	private:
		std::vector<Triangle *> m_triangles;
		Material *m_material;
		bool m_invertNormals;
	};
}