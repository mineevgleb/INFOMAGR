#include "Mesh.h"
#include <assimp/Importer.hpp>    
#include <assimp/scene.h>           
#include <assimp/postprocess.h>
#include "../util.h"
#include <vector>

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

	bool Mesh::load(const std::string& path, NormalType nt)
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
		std::vector<std::vector<int>> facesForVertex(mesh->mNumVertices);
		std::vector<std::vector<int>> numInFaceForVertex(mesh->mNumVertices);

		for (size_t i = 0; i < mesh->mNumFaces; ++i) {
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
			m_triangles.push_back(new Triangle(a, b, c, *m_material, hasTexCoord, 
				hasNormals & (nt != FLAT)));
			if (nt == CONSISTENT) {
				facesForVertex[mesh->mFaces[i].mIndices[0]].push_back(i);
				facesForVertex[mesh->mFaces[i].mIndices[1]].push_back(i);
				facesForVertex[mesh->mFaces[i].mIndices[2]].push_back(i);
				numInFaceForVertex[mesh->mFaces[i].mIndices[0]].push_back(0);
				numInFaceForVertex[mesh->mFaces[i].mIndices[1]].push_back(1);
				numInFaceForVertex[mesh->mFaces[i].mIndices[2]].push_back(2);
			}
		}

		if (nt == CONSISTENT) {
			for (int i = 0; i < facesForVertex.size(); ++i) {
				glm::vec3 vertNormal;
				assimpVec2glmVec(mesh->mNormals[i], vertNormal);
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
		importer.FreeScene();
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
