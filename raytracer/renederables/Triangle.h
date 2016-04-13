#pragma once
#include "Primitive.h"

namespace AGR {
	struct Vertex
	{
		Vertex(const glm::vec3& position = glm::vec3(0,0,0),  
			const glm::vec2& texCoord = glm::vec2(0, 0),
			const glm::vec3& normal = glm::vec3(0, 1, 0)) :
			position(position),
			texCoord(texCoord),
			normal(normal),
			alpha(-1.0f)
		{}
		glm::vec3 position;
		glm::vec2 texCoord;
		glm::vec3 normal;
		float alpha; //used for consistent normals
	};


	class Triangle : public Primitive {
	public:
		//when limit is false triangle becomes a plane
		Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3,
		         Material& m, bool useTextureCoords = true, 
				 bool useVertNormals = false, bool limit = true);


		float intersect(const Ray &r) const override;
		void getTexCoordAndNormal(const Ray& r, float dist,
			glm::vec2& texCoord, glm::vec3& normal) const override;

		void setVertex(int num, const Vertex& val);

		const Vertex& getVertex(int num) const;

		void setUseTextureCoords(bool val);

		bool getUseTextureCoords() const;

		void setUseVertNormals(bool val);

		bool getUseVertNormals() const;

		const glm::vec3& getFaceNormal() const;

		void commitTransformations();
		float getArea() override;
	private:
		bool calcBarycentricCoord(const glm::vec3& pt, glm::vec3& out, bool limit) const;
		Vertex m_vert[3];
		glm::vec3 m_v0v1;
		glm::vec3 m_v0v2;
		glm::vec3 m_normal;
		bool m_useTextureCoords;
		bool m_useVertNormals;
		bool m_limit;
		float m_area;

		//for barycentric coord
		float m_d00, m_d01, m_d11, m_invdenom;
	};

}