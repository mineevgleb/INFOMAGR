#include "Mesh.h"
#include <assimp/Importer.hpp>    
#include <assimp/scene.h>           
#include <assimp/postprocess.h>

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
			a.position /= 44;
			b.position /= 44;
			c.position /= 44;
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
			m_triangles.push_back(new Triangle(a, b, c, *m_material, m_invertNormals, hasTexCoord, hasNormals));
		}

		return true;
	}

	void Mesh::release()
	{
		for (int i = 0; i < m_triangles.size(); ++i)
			delete m_triangles[i];
		m_triangles.clear();
	}
}
