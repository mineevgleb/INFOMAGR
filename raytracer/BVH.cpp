#include "BVH.h"
#include "util.h"
#include <ppl.h>

namespace AGR
{

	void BVH::construct(std::vector<Primitive*>& primitives)
	{
		m_primitives = primitives;
		sortPrimitivesByMortonCodes();
		m_nodes.resize(m_primitives.size() * 2);
		m_nodesCount = 1;
		std::vector<NodePair> nodes(m_primitives.size());
		for (int i = 0; i < m_primitives.size(); ++i) {
			nodes[i] = { i + 1, nullptr, FLT_MAX };
			m_nodes[m_nodesCount].primitiveNum = i;
			m_nodes[m_nodesCount].bounds = m_primitives[i]->getBoundingBox();
			m_nodes[m_nodesCount].isLeaf |= Node::LEAF_FLAG;
			++m_nodesCount;
		}
		int newSize = BuildTreeAgglomerative(&nodes[0], static_cast<int>(m_primitives.size()));
		int curPos = 0;
		for (int i = 0; i < m_primitives.size(); ++i) {
			if (nodes[i].nodeNum > 0) {
				nodes[curPos] = nodes[i];
				if (i != curPos) nodes[i].nodeNum = -1;
				curPos++;
			}
		}
		combineClusters(&nodes[0], newSize, 1);
		m_nodes[0] = m_nodes[--m_nodesCount];
		m_quadNodes.resize(m_nodes.size());
		m_quadNodesCount = 1;
		Node *initialChildren[4];
		formQuadNode(&m_nodes[0], initialChildren);
		buildQuadTree(&m_quadNodes[0], initialChildren);
	}

	bool BVH::Traverse(Ray& ray, Intersection& intersect, float minLength)
	{
		intersect.ray_length = -1.0f;
		glm::vec3 invDirection = 1.0f / ray.direction;
		float dist;
		if (!m_nodes[0].bounds.intersect(ray, dist, invDirection)) {
			return false;
		}
		RaySIMD rsimd;
		rsimd.invdirx4 = _mm_load1_ps(&invDirection.x);
		rsimd.invdiry4 = _mm_load1_ps(&invDirection.y);
		rsimd.invdirz4 = _mm_load1_ps(&invDirection.z);
		rsimd.origx4 = _mm_load1_ps(&ray.origin.x);
		rsimd.origy4 = _mm_load1_ps(&ray.origin.y);
		rsimd.origz4 = _mm_load1_ps(&ray.origin.z);
		if(Traverse(ray, rsimd, intersect, &m_quadNodes[0], minLength)) {
			return true;
		}
		return false;
	}

	void BVH::PacketCheckOcclusions(std::vector<Ray>& rays, std::vector<float>& lengths,
		std::vector<bool>& occlusionFlags)
	{
		const int chunksAm = 32;
		concurrency::parallel_for(0, chunksAm, 1,
			[this, &rays, &lengths, &occlusionFlags, &chunksAm](int iter) {
			int from = ((rays.size() / chunksAm) + 1) * iter;
			int to = ((rays.size() / chunksAm) + 1) * (iter + 1);
			if (to > rays.size()) to = rays.size();
			for (int i = from; i < to; ++i) {
				Intersection intersect;
				intersect.ray_length = -1.0f;
				Traverse(rays[i], intersect, lengths[i]);
				occlusionFlags[i] = intersect.ray_length > 0 && intersect.ray_length < lengths[i];
			}
		});
	}

	void BVH::PacketTraverse(std::vector<Ray>& rays, std::vector<Intersection>& intersect)
	{
		intersect.resize(rays.size());
		const int chunksAm = 64;
		concurrency::parallel_for(0, chunksAm, 1,
			[this, &intersect, &rays, &chunksAm](int iter) {
			int from = ((rays.size() / chunksAm) + 1) * iter;
			int to = ((rays.size() / chunksAm) + 1) * (iter + 1);
			if (to > rays.size()) to = rays.size();
			for (int i = from; i < to; ++i) {
				Traverse(rays[i], intersect[i], -1.0f);
			}
		});
	}

	__m128 BVH::QuadNode::intersect(RaySIMD& r, __m128& dist) const
	{
		union {
			__m128 zero4;
			float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		};
		__m128 t1x = _mm_sub_ps(minx4, r.origx4);
		t1x = _mm_mul_ps(t1x, r.invdirx4);
		__m128 t1y = _mm_sub_ps(miny4, r.origy4);
		t1y = _mm_mul_ps(t1y, r.invdiry4);
		__m128 t1z = _mm_sub_ps(minz4, r.origz4);
		t1z = _mm_mul_ps(t1z, r.invdirz4);
		__m128 t2x = _mm_sub_ps(maxx4, r.origx4);
		t2x = _mm_mul_ps(t2x, r.invdirx4);
		__m128 t2y = _mm_sub_ps(maxy4, r.origy4);
		t2y = _mm_mul_ps(t2y, r.invdiry4);
		__m128 t2z = _mm_sub_ps(maxz4, r.origz4);
		t2z = _mm_mul_ps(t2z, r.invdirz4);
		__m128 xmin = _mm_min_ps(t1x, t2x);
		__m128 ymin = _mm_min_ps(t1y, t2y);
		__m128 zmin = _mm_min_ps(t1z, t2z);
		__m128 xmax = _mm_max_ps(t1x, t2x);
		__m128 ymax = _mm_max_ps(t1y, t2y);
		__m128 zmax = _mm_max_ps(t1z, t2z);
		dist = _mm_max_ps(_mm_max_ps(xmin, ymin), zmin);
		__m128 tmax = _mm_min_ps(_mm_min_ps(xmax, ymax), zmax);
		return _mm_and_ps(_mm_cmpgt_ps(tmax, dist), _mm_cmpgt_ps(tmax, zero4));
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
		int commonPrefix = lzcnt64(diff);
		int split = 0; 
		int step = size - 1;
		do
		{
			step = (step + 1) >> 1;
			int newSplit = split + step;
			if (newSplit < size)
			{
				::uint64_t splitCode = primitives[newSplit]->m_mortonCode;
				int splitPrefix = lzcnt64(first ^ splitCode);
				if (splitPrefix > commonPrefix)
					split = newSplit;
			}
		} while (step > 1);
		return split + 1;
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

	void BVH::sortPrimitivesByMortonCodes()
	{
		AABB surround = getSurroundAABB(&m_primitives[0], m_primitives.size());
		glm::vec3 surroundMin = surround.getMinPt();
		glm::vec3 surroundMax = surround.getMaxPt();
		for (int i = 0; i < m_primitives.size(); ++i) {
			glm::vec3 bbCenter = m_primitives[i]->getBoundingBox().getCenter();
			m_primitives[i]->m_mortonCode = CalcMortonCode(bbCenter, surroundMin, surroundMax);
		}
		concurrency::parallel_buffered_sort(m_primitives.begin(), m_primitives.end(),
			[](Primitive *a, Primitive *b)->bool {return a->m_mortonCode < b->m_mortonCode;});
	}

	bool BVH::Traverse(Ray& ray, RaySIMD& rsimd, Intersection& intersect, QuadNode *node, float minLength)
	{
		union
		{
			__m128 dist4;
			float dist[4];
		};
		union
		{
			__m128 intersectFlags4;
			unsigned int intersectFlags[4];
		};
		intersectFlags4 = node->intersect(rsimd, dist4);
		bool wasHit = false;
		for (int i = 0; i < 4; ++i) {
			if (intersectFlags[i] && (intersect.ray_length < 0 || dist[i] < intersect.ray_length)) {
				if (node->isLeaf[i] & QuadNode::LEAF_FLAG) {
					Primitive* obj = m_primitives[node->child[i] & (~QuadNode::LEAF_FLAG)];
					float rayLen = obj->intersect(ray);
					if (rayLen > 0) {
						if (intersect.ray_length < 0 || rayLen < intersect.ray_length) {
							intersect.ray_length = rayLen;
							intersect.p_object = obj;
							wasHit = true;
						}
					}
				} else {
					wasHit |= Traverse(ray, rsimd, intersect, 
						&m_quadNodes[node->child[i] & (~QuadNode::LEAF_FLAG)], minLength);
				}
				if (intersect.ray_length > 0.0f && intersect.ray_length < minLength) return true;
			}
		}
		return wasHit;
	}

	void BVH::buildQuadTree(QuadNode * parent, Node ** children)
	{
		memset(parent, 0, sizeof(*parent));
		for (int i = 0; i < 4; ++i) {
			if (children[i]) {
				glm::vec3 minpt = children[i]->bounds.getMinPt();
				glm::vec3 maxpt = children[i]->bounds.getMaxPt();
				parent->minx[i] = minpt.x;
				parent->miny[i] = minpt.y;
				parent->minz[i] = minpt.z;
				parent->maxx[i] = maxpt.x;
				parent->maxy[i] = maxpt.y;
				parent->maxz[i] = maxpt.z;
				if (children[i]->isLeaf & Node::LEAF_FLAG) {
					parent->child[i] = children[i]->primitiveNum;
				} else {
					parent->child[i] = m_quadNodesCount;
					Node *newChildren[4];
					formQuadNode(children[i], newChildren);
					buildQuadTree(&m_quadNodes[m_quadNodesCount++], newChildren);
				}
				parent->isLeaf[i] |= children[i]->isLeaf & Node::LEAF_FLAG;
			} 
		}
	}

	void BVH::formQuadNode(Node* parent, Node **children)
	{
		memset(children, 0, sizeof(children[0]) * 4);
		int curAm = 2;
		children[0] = &m_nodes[parent->left];
		children[1] = &m_nodes[parent->right];
		bool modified = false;
		do {
			modified = false;
			for (int i = 0; i < curAm && curAm < 4; ++i) {
				if (!(children[i]->isLeaf & Node::LEAF_FLAG)) {
					modified = true;
					children[curAm++] = &m_nodes[children[i]->right];
					children[i] = &m_nodes[children[i]->left];
				}
			}
		} while (modified);
	}

	int BVH::BuildTreeAgglomerative(NodePair *nodesArr, int size)
	{
		int curPos = 0;
		if (size <= CLUSTER_SIZE) {
			int clusterSize = calcClusterSize(size);
			combineClusters(nodesArr, size, clusterSize);
			for (int i = 0; i < size; ++i) {
				if (nodesArr[i].nodeNum > 0) {
					nodesArr[curPos] = nodesArr[i];
					if (i != curPos) nodesArr[i].nodeNum = -1;
					curPos++;
				}
			}
			return clusterSize;
		}
		int firstPrimitiveIdx = m_nodes[nodesArr->nodeNum].primitiveNum;
		size_t split = findSplit(&m_primitives[firstPrimitiveIdx], size);
		int newSize = BuildTreeAgglomerative(nodesArr, split);
		newSize += BuildTreeAgglomerative(&nodesArr[split], size - split);
		for (int i = 0; i < size; ++i) {
			if (nodesArr[i].nodeNum > 0) {
				nodesArr[curPos] = nodesArr[i];
				if (i != curPos) nodesArr[i].nodeNum = -1;
				curPos++;
			}
		}
		int clusterSize = calcClusterSize(newSize);
		combineClusters(nodesArr, newSize, clusterSize);
		return clusterSize;
	}

	void BVH::combineClusters(NodePair *nodesArr, int size, int amount)
	{
		int curSize = size;
		for (int i = 0; i < size; ++i) {
			findBestMatch(&nodesArr[i], nodesArr, size);
		}
		while (curSize > amount) {
			NodePair *bestPair = nodesArr;
			for (int i = 1; i < size; ++i) {
				if (nodesArr[i].dist < bestPair->dist) {
					bestPair = &nodesArr[i];
				}
			}
			m_nodes[m_nodesCount].left = bestPair->nodeNum;
			m_nodes[m_nodesCount].right = bestPair->closestNode->nodeNum;
			m_nodes[m_nodesCount].bounds = m_nodes[bestPair->nodeNum].bounds;
			m_nodes[m_nodesCount].bounds.extend(m_nodes[bestPair->closestNode->nodeNum].bounds);
			NodePair* deleted1 = bestPair->closestNode;
			NodePair* deleted2 = bestPair;
			*bestPair->closestNode = { -1, nullptr, FLT_MAX };
			*bestPair = { m_nodesCount, nullptr, FLT_MAX };
			findBestMatch(bestPair, nodesArr, size);
			for (int i = 0; i < size; ++i) {
				if (nodesArr[i].closestNode == deleted1 ||
					nodesArr[i].closestNode == deleted2) {
					findBestMatch(&nodesArr[i], nodesArr, size);
				}
			}
			++m_nodesCount;
			--curSize;
		}
	}

	void BVH::findBestMatch(NodePair* node, NodePair *nodesArr, int count)
	{
		node->dist = FLT_MAX;
		for (int i = 0; i < count; ++i) {
			if (node != &nodesArr[i] && nodesArr[i].nodeNum != -1) {
				AABB united = m_nodes[node->nodeNum].bounds;
				united.extend(m_nodes[nodesArr[i].nodeNum].bounds);
				float curDist = united.calcArea();
				if (curDist < node->dist) {
					node->dist = curDist;
					node->closestNode = &nodesArr[i];
				}
			}
		}
	}

	int BVH::calcClusterSize(int amountOfNodes) const
	{
		const float scaler = pow(CLUSTER_SIZE, 0.5f + CLUSTERFUNC_EPSILON) / 2;
		int am = round(scaler * pow(amountOfNodes, 0.5f - CLUSTERFUNC_EPSILON));
		return am <= amountOfNodes ? am : amountOfNodes;
	}
}
