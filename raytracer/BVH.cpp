#include "BVH.h"
#include "util.h"
#include <algorithm>

/*
TODO
+Morton codes
+Sort
Hierarchy
Traversal
Check
Parallelize
	Understand OCL
	Parallelize build
	Parallelize traverse
Understand 2nd paper
*/

namespace AGR
{
	void BVH::construct(std::vector<Primitive*>& primitives)
	{
		m_primitives = &primitives;
		AABB surround = getSurroundAABB(&primitives[0], primitives.size());
		glm::vec3 surroundMin = surround.getMinPt();
		glm::vec3 surroundMax = surround.getMaxPt();
		for (int i = 0; i < primitives.size(); ++i) {
			glm::vec3 bbCenter = primitives[i]->getBoundingBox().getCenter();
			primitives[i]->m_mortonCode = CalcMortonCode(bbCenter, surroundMin, surroundMax);
		}
		std::sort(primitives.begin(), primitives.end(),
			[](Primitive *a, Primitive *b)->bool{return a->m_mortonCode < b->m_mortonCode;});
		m_nodesCount = 1;
		m_nodes.resize(primitives.size() * 2 - 1);
		m_nodes[0].count = primitives.size();
		Subdivide(0, &primitives[0], 0);
		calcAABB(0);
	}

	bool BVH::Traverse(const Ray& ray, Intersection& intersect)
	{
		glm::vec3 invRayDir = 1.0f / ray.direction;
		if(Traverse(ray, intersect, 0, invRayDir)) {
			intersect.p_object->getTexCoordAndNormal(intersect);
			return true;
		}
		return false;
	}

	AABB BVH::getSurroundAABB(Primitive** primitives, size_t size) const
	{
		if (size == 1) {
			return primitives[0]->getBoundingBox();
		}
		if (size == 2) {
			AABB bb = primitives[0]->getBoundingBox();
			bb.extend(primitives[1]->getBoundingBox());
			return bb;
		}
		AABB bb = getSurroundAABB(primitives, size / 2);
		bb.extend(getSurroundAABB(&primitives[size / 2], size - size / 2));
		return bb;
	}

	size_t BVH::findSplit(Primitive** primitives, size_t size)
	{
		::uint64_t first = primitives[0]->m_mortonCode;
		::uint64_t last = primitives[size - 1]->m_mortonCode;
		if (first == last) return size / 2;
		::uint64_t diff = first ^ last;
		int commonPrefix = lzcnt(diff);
		int split = 0; 
		int step = size - 1;
		do
		{
			step = (step + 1) >> 1;
			int newSplit = split + step;
			if (newSplit < size)
			{
				::uint64_t splitCode = primitives[newSplit]->m_mortonCode;
				int splitPrefix = lzcnt(first ^ splitCode);
				if (splitPrefix > commonPrefix)
					split = newSplit;
			}
		} while (step > 1);
		return split + 1;
	}

	void BVH::Subdivide(int nodeNum, Primitive** primitives, int first)
	{
		depth++;
		if (depth > maxdepth) maxdepth = depth;
		if (m_nodes[nodeNum].count == 1) {
			m_nodes[nodeNum].leftFirst = first;
			depth--;
			return;
		}
		m_nodes[nodeNum].leftFirst = m_nodesCount;
		size_t split = findSplit(primitives, m_nodes[nodeNum].count);
		m_nodesCount += 2;
		m_nodes[m_nodes[nodeNum].leftFirst].count = split;
		m_nodes[m_nodes[nodeNum].leftFirst + 1].count = m_nodes[nodeNum].count - split;
		Subdivide(m_nodes[nodeNum].leftFirst, primitives, first);
		Subdivide(m_nodes[nodeNum].leftFirst + 1, &primitives[split], first + split);
		depth--;
	}

	void BVH::calcAABB(int nodeNum)
	{
		if (m_nodes[nodeNum].count == 1) {
			m_nodes[nodeNum].bounds = (*m_primitives)[m_nodes[nodeNum].leftFirst]->getBoundingBox();
			return;
		}
		calcAABB(m_nodes[nodeNum].leftFirst);
		calcAABB(m_nodes[nodeNum].leftFirst + 1);
		m_nodes[nodeNum].bounds = m_nodes[m_nodes[nodeNum].leftFirst].bounds;
		m_nodes[nodeNum].bounds.extend(m_nodes[m_nodes[nodeNum].leftFirst + 1].bounds);
	}

	// Expands a 20-bit integer into 60 bits
	// by inserting 2 zeros after each bit.
	uint64_t BVH::expandBits(::uint64_t v) const
	{
		v = (v * UINT64_C(0x0000000100000001)) & UINT64_C(0xFFFF00000000FFFF);
		v = (v * UINT64_C(0x0000000000010001)) & UINT64_C(0x00FF0000FF0000FF);
		v = (v * UINT64_C(0x0000000000000101)) & UINT64_C(0xF00F00F00F00F00F);
		v = (v * UINT64_C(0x0000000000000011)) & UINT64_C(0x30C30C30C30C30C3);
		v = (v * UINT64_C(0x0000000000000005)) & UINT64_C(0x9249249249249249);
		return v;
	}

	uint64_t BVH::CalcMortonCode(glm::vec3& pt, glm::vec3& min, glm::vec3& max) const
	{
		glm::vec3 normalized = (pt - min) / (max - min);
		float x = fmin(fmax(normalized.x * 1048576.0f, 0.0f), 1048575.0f);
		float y = fmin(fmax(normalized.y * 1048576.0f, 0.0f), 1048575.0f);
		float z = fmin(fmax(normalized.z * 1048576.0f, 0.0f), 1048575.0f);
		::uint64_t xx = expandBits(static_cast<::uint64_t>(x));
		::uint64_t yy = expandBits(static_cast<::uint64_t>(y));
		::uint64_t zz = expandBits(static_cast<::uint64_t>(z));
		return zz | (yy << 1) | (xx << 2);
	}

	bool BVH::Traverse(const Ray& ray, Intersection& intersect, int nodeNum, glm::vec3& invRayDir)
	{
		bool inters = m_nodes[nodeNum].bounds.testIntersection(ray, invRayDir);
		if (!inters) return false;
		if (m_nodes[nodeNum].count == 1) {
			Intersection test;
			test.ray = ray;
			if ((*m_primitives)[m_nodes[nodeNum].leftFirst]->intersect(test)) {
				if (test.ray_length > 0 && test.ray_length < intersect.ray_length) {
					intersect = test;
					return true;
				}
			}
			return false;
		}
		bool t1 = Traverse(ray, intersect, m_nodes[nodeNum].leftFirst, invRayDir);
		bool t2 = Traverse(ray, intersect, m_nodes[nodeNum].leftFirst + 1, invRayDir);
		return t1 || t2;
	}
}
