#pragma once
#include "Renderable.h" 
#include "Triangle.h"
#include <vector>
#include <string>

namespace AGR {
	class MeshTriangle;

	class Mesh : public Renderable
	{
	public:
		Mesh(Material& m, bool invertNormals) 
			: Renderable(m, invertNormals) {}
		~Mesh() { release(); }
		bool intersect(const Ray& r, Intersection& out) const override;
		void getTexCoord(glm::vec3 pt, glm::vec2& out) const override {}
		bool load(const std::string &path);
		void release();
	private:
		std::vector<MeshTriangle *> m_triangles;
	};

	//For correct refractions and texturing
	class MeshTriangle : public Renderable
	{
	public:
		MeshTriangle (Triangle* t, const Mesh* m, Material &mat) : 
			Renderable(mat), t(t), m(m) {}
		~MeshTriangle() { delete t; }
		bool intersect(const Ray& r, Intersection& out) const override
		{
			return m->intersect(r, out);
		}
		bool intersectWithTriangle(const Ray& r, Intersection& out) const
		{
			return t->intersect(r, out);
		}
		void getTexCoord(glm::vec3 pt, glm::vec2& out) const override
		{
			return t->getTexCoord(pt, out);
		}
	private:
		Triangle* t;
		const Mesh* m;
	};
}