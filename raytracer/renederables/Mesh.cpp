#include "Mesh.h"
#include <tiny_obj_loader.h>    
#include "../util.h"
#include <vector>

namespace AGR
{
	bool Mesh::load(const std::string& path, NormalType nt)
	{
		std::vector<tinyobj::material_t> materials;
		std::vector<tinyobj::shape_t> shapes;
		std::string err;
		LoadObj(shapes, materials, err, path.c_str());
		if (shapes.empty()) return false;
		if (shapes[0].mesh.indices.empty()) return false;
		if (shapes[0].mesh.normals.empty()) nt = FLAT;
		bool hasTexCoord = !shapes[0].mesh.texcoords.empty();
		std::vector<std::vector<int>> facesForVertex(shapes[0].mesh.positions.size() / 3);
		std::vector<std::vector<int>> numInFaceForVertex(shapes[0].mesh.positions.size() / 3);
		for (size_t i = 0; i < shapes[0].mesh.indices.size() / 3; ++i) {
			Vertex a, b, c;
			float *tmp;
			tmp = &shapes[0].mesh.positions[shapes[0].mesh.indices[i * 3] * 3];
			a.position = glm::vec3(tmp[0], tmp[1], tmp[2]);
			tmp = &shapes[0].mesh.positions[shapes[0].mesh.indices[i * 3 + 1] * 3];
			b.position = glm::vec3(tmp[0], tmp[1], tmp[2]);
			tmp = &shapes[0].mesh.positions[shapes[0].mesh.indices[i * 3 + 2] * 3];
			c.position = glm::vec3(tmp[0], tmp[1], tmp[2]);
			if (nt != FLAT) {
				tmp = &shapes[0].mesh.normals[shapes[0].mesh.indices[i * 3] * 3];
				a.normal = glm::normalize(glm::vec3(tmp[0], tmp[1], tmp[2]));
				tmp = &shapes[0].mesh.normals[shapes[0].mesh.indices[i * 3 + 1] * 3];
				b.normal = glm::normalize(glm::vec3(tmp[0], tmp[1], tmp[2]));
				tmp = &shapes[0].mesh.normals[shapes[0].mesh.indices[i * 3 + 2] * 3];
				c.normal = glm::normalize(glm::vec3(tmp[0], tmp[1], tmp[2]));
			}

			if (hasTexCoord) {
				tmp = &shapes[0].mesh.texcoords[shapes[0].mesh.indices[i * 3] * 2];
				a.texCoord = glm::vec2(tmp[0], tmp[1]);
				tmp = &shapes[0].mesh.texcoords[shapes[0].mesh.indices[i * 3 + 1] * 2];
				b.texCoord = glm::vec2(tmp[0], tmp[1]);
				tmp = &shapes[0].mesh.texcoords[shapes[0].mesh.indices[i * 3 + 2] * 2];
				c.texCoord = glm::vec2(tmp[0], tmp[1]);
			}
			m_triangles.push_back(new Triangle(a, b, c, *m_material, hasTexCoord, nt != FLAT));
			if (nt == CONSISTENT) {
				facesForVertex[shapes[0].mesh.indices[i * 3]].push_back(i);
				facesForVertex[shapes[0].mesh.indices[i * 3 + 1]].push_back(i);
				facesForVertex[shapes[0].mesh.indices[i * 3 + 2]].push_back(i);
				numInFaceForVertex[shapes[0].mesh.indices[i * 3]].push_back(0);
				numInFaceForVertex[shapes[0].mesh.indices[i * 3 + 1]].push_back(1);
				numInFaceForVertex[shapes[0].mesh.indices[i * 3 + 2]].push_back(2);
			}
		}

		if (nt == CONSISTENT) {
			for (int i = 0; i < facesForVertex.size(); ++i) {
				float *tmp = &shapes[0].mesh.normals[i * 3];
				glm::vec3 vertNormal(tmp[0], tmp[1], tmp[2]);
				vertNormal = glm::normalize(vertNormal);
				float minCos = 2.0f;
				for (int facenum : facesForVertex[i]) {
					float curCos = glm::dot(vertNormal, m_triangles[facenum]->getFaceNormal());
					if (curCos < minCos) minCos = curCos;
				}
				for (int j = 0; j < facesForVertex[i].size(); ++j) {
					int faceNum = facesForVertex[i][j];
					int vertNum = numInFaceForVertex[i][j];
					Vertex v = m_triangles[faceNum]->getVertex(vertNum);
					v.alpha = glm::acos(minCos) * (1.0f + 0.03632f * (1 - minCos) * (1 - minCos));
					m_triangles[faceNum]->setVertex(vertNum, v);
				}
			}
			for (auto& t:m_triangles) {
				t->commitTransformations();
			}
		}
		return true;
	}

	void Mesh::setPosition(const glm::vec3& p)
	{
		m_translation = p;
	}

	void Mesh::setRotation(const glm::vec3& r)
	{
		m_rotation = r;
	}

	void Mesh::setScale(const glm::vec3& s)
	{
		m_scale = s;
	}

	void Mesh::release()
	{
		for (int i = 0; i < m_triangles.size(); ++i)
			delete m_triangles[i];
		m_triangles.clear();
	}

	void Mesh::commitTransformations()
	{
		glm::mat4x4 newModMatrix;
		glm::mat4x4 newNormModMatrix;
		transformMat(m_translation, m_rotation, m_scale, newModMatrix);
		transformMat(glm::vec3(), m_rotation, 1.0f / m_scale, newNormModMatrix);
		glm::mat4x4 invModMatrix = glm::inverse(m_modMatrix);
		glm::mat4x4 invNormModMatrix = glm::inverse(m_normModMatrix);
		glm::mat4x4 combinedMatrix = newModMatrix * invModMatrix;
		glm::mat4x4 combinedNormMatrix = newNormModMatrix * invNormModMatrix;
		m_modMatrix = newModMatrix;
		m_normModMatrix = newNormModMatrix;
		for (Triangle * t : m_triangles) {
			for (int i = 0; i < 3; ++i) {
				Vertex v = t->getVertex(i);
				glm::vec4 newVec = glm::vec4(v.position, 1);
				newVec = combinedMatrix * newVec;
				v.position = glm::vec3(newVec);
				newVec = glm::vec4(v.normal, 1);
				newVec = combinedNormMatrix * newVec;
				v.normal = glm::normalize(glm::vec3(newVec));
				t->setVertex(i, v);
			}
			t->commitTransformations();
		}
	}
}
