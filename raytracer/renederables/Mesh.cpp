#include "Mesh.h"
#include <assimp/Importer.hpp>    
#include <assimp/scene.h>           
#include <assimp/postprocess.h>
#include "../util.h"

namespace AGR
{
	void assimpVec2glmVec(const aiVector3D& from, glm::vec3& to)
	{
		to.x = from.x;
		to.y = from.y;
		to.z = from.z;
	}

	void assimpVec2glmVec(const aiVector3D& from, glm::vec2& to)
	{
		to.x = from.x;
		to.y = from.y;
	}

	bool Mesh::load(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene *scene =
			importer.ReadFile(path.c_str(),
				aiProcessPreset_TargetRealtime_MaxQuality |
				aiProcess_TransformUVCoords
				);
		if (!scene || ! scene->HasMeshes()) return false;
		const aiMesh* mesh = scene->mMeshes[0];
		if (!mesh->HasFaces()) return false;
		bool hasNormals = mesh->HasNormals();
		bool hasTexCoord = mesh->HasTextureCoords(0);
		for (int i = 0; i < mesh->mNumFaces; ++i) {
			Vertex a, b, c;
			assimpVec2glmVec
				(mesh->mVertices[mesh->mFaces[i].mIndices[0]], a.position);
			assimpVec2glmVec
				(mesh->mVertices[mesh->mFaces[i].mIndices[1]], b.position);
			assimpVec2glmVec
				(mesh->mVertices[mesh->mFaces[i].mIndices[2]], c.position);
			if (hasNormals) {
				assimpVec2glmVec
					(mesh->mNormals[mesh->mFaces[i].mIndices[0]], a.normal);
				assimpVec2glmVec
					(mesh->mNormals[mesh->mFaces[i].mIndices[1]], b.normal);
				assimpVec2glmVec
					(mesh->mNormals[mesh->mFaces[i].mIndices[2]], c.normal);
			}

			if (hasTexCoord) {
				assimpVec2glmVec
					(mesh->mTextureCoords[0][mesh->mFaces[i].mIndices[0]], a.texCoord);
				assimpVec2glmVec
					(mesh->mTextureCoords[0][mesh->mFaces[i].mIndices[1]], b.texCoord);
				assimpVec2glmVec
					(mesh->mTextureCoords[0][mesh->mFaces[i].mIndices[2]], c.texCoord);
			}
			m_triangles.push_back(new Triangle(a, b, c, *m_material, 
				m_invertNormals, hasTexCoord, hasNormals & m_isSmooth));
		}

		return true;
	}

	void Mesh::setPosition(const glm::vec3& p, bool update)
	{
		m_translation = p;
		if (update) updateTriangles();
	}

	void Mesh::setRotation(const glm::vec3& r, bool update)
	{
		m_rotation = r;
		if (update) updateTriangles();
	}

	void Mesh::setScale(const glm::vec3& s, bool update)
	{
		m_scale = s;
		if (update) updateTriangles();
	}

	void Mesh::release()
	{
		for (int i = 0; i < m_triangles.size(); ++i)
			delete m_triangles[i];
		m_triangles.clear();
	}

	void Mesh::updateTriangles()
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
		}
	}
}
