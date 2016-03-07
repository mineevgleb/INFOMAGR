#include "BVH.h"

namespace AGR
{
	void BVH::construct(std::vector<const Primitive*>& primitives)
	{
		m_nodesCount = 1;
		m_nodes.resize(primitives.size() * 2 - 1);
		m_nodes[0].count = primitives.size();
	}

	void BVH::Subdivide(int nodeNum)
	{
		if (m_nodes[nodeNum].count < 3) return;
		m_nodes[nodeNum].leftFirst = m_nodesCount;
		m_nodesCount += 2;
	}
}
