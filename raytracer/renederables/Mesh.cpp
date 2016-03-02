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

	bool Mesh::intersect(const Ray& r, Intersection& out) const
	{
		out.ray_length = FLT_MAX;
		Intersection tmp;
		bool wasIntersect = false;;
		for (int i = 0; i < m_triangles.size(); ++i) {
			if (m_triangles[i]->intersectWithTriangle(r, tmp) && 
				tmp.ray_length < out.ray_length) {
				out = tmp;
				out.p_object = m_triangles[i];
				wasIntersect = true;
			}
		}
		return wasIntersect;
	}

	bool Mesh::load(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene *scene =
			importer.ReadFile(path.c_str(),
				aiProcess_JoinIdenticalVertices|
				aiProcess_Triangulate|
				aiProcess_GenNormals|
				aiProcess_PreTransformVertices|
				aiProcess_FixInfacingNormals |
				aiProcess_SortByPType |
				aiProcess_GenUVCoords |
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
					(mesh->mTextureCoords[0][mesh->mFaces[i].mIndices[2]], b.texCoord);
			}
			Triangle* t = new Triangle(a, b, c, *m_material, m_invertNormals, hasTexCoord, hasNormals);
			m_triangles.push_back(new MeshTriangle(t, this, *m_material));
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
