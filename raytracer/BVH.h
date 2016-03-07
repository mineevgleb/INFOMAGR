#pragma once
#include <vector>
#include "renederables/Primitive.h"
#include "AABB.h"

namespace AGR
{
	struct Node
	{
		AABB bounds;
		int leftFirst;
		int count;
	};

	class BVH
	{
	public:
		void construct(std::vector<const Primitive *>& primitives);
		void release();
	private:
		void Subdivide(int nodeNum);
		std::vector<Node> m_nodes;
		int m_nodesCount;
	};
}
