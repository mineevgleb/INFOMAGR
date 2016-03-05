#pragma once
#include "Renderable.h"

namespace AGR {
	struct Vertex
	{
		Vertex(const glm::vec3& position = glm::vec3(0,0,0),  
			const glm::vec2& texCoord = glm::vec2(0, 0),
			const glm::vec3& normal = glm::vec3(0, 1, 0)) :
			position(position),
			texCoord(texCoord),
			normal(normal)
		{}
		glm::vec3 position;
		glm::vec2 texCoord;
		glm::vec3 normal;
	};


	class Triangle : public Renderable {
	public:
		Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3,
		         Material& m, bool invertNormals = false,
		         bool useTextureCoords = false, bool useVertNormals = false);


		bool intersect(const Ray &r, Intersection &out) const override;
		void getTexCoord(glm::vec3 pt, glm::vec2& out) const override;

		void setVertex(int num, const Vertex& val);

		const Vertex& getVertex(int num) const;

		void setUseTextureCoords(bool val);

		bool getUseTextureCoords() const;

		void setUseVertNormals(bool val);

		bool getUseVertNormals() const;
	private:
		bool calcBarycentricCoord(const glm::vec3& pt, glm::vec3& out) const;
		void recalcInternalInfo();
		Vertex m_vert[3];
		glm::vec3 m_v0v1;
		glm::vec3 m_v0v2;
		glm::vec3 m_normal;
		bool m_useTextureCoords;
		bool m_useVertNormals;

		//for barycentric coord
		float m_d00, m_d01, m_d11, m_invdenom;
	};

}