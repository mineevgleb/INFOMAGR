#pragma once
#include <vector>
#include "renederables/Primitive.h"
#include "AABB.h"

namespace AGR
{

	class BVH
	{
	public:
		void construct(std::vector<Primitive *>& primitives);
		bool Traverse(const Ray& ray, Intersection& intersect);
	private:
		struct Node
		{
			AABB bounds;
			int leftFirst;
			int count;
		};
		AABB getSurroundAABB(Primitive** primitives, size_t size) const;
		size_t findSplit(Primitive** primitives, size_t size);
		void Subdivide(int nodeNum, Primitive** primitives, int first);
		void calcAABB(int nodeNum);
		::uint64_t expandBits(::uint64_t v) const;
		::uint64_t CalcMortonCode(glm::vec3& pt, glm::vec3& min, glm::vec3& max) const;
		bool Traverse(const Ray& ray, Intersection& intersect, int nodeNum, glm::vec3& invRayDir);

		std::vector<Primitive *>* m_primitives;
		std::vector<Node> m_nodes;
		int m_nodesCount;
		int depth = 0;
		int maxdepth = 0;
	};
}
