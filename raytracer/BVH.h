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
		bool Traverse(Ray& ray, Intersection& intersect, int minLength);
		void PacketTraverse(std::vector<Ray>& rays, std::vector<Intersection>& intersect);
		void PacketCheckOcclusions(std::vector<Ray>& rays, 
			std::vector<float>& lengths, std::vector<bool>& occlusionFlags);
	private:
		struct Node
		{
			AABB bounds;
			int leftFirst;
			int right;
			int count;
			int parent;
			bool isLeaf;
		};

		struct RaySIMD
		{
			__m128 origx4;
			__m128 origy4;
			__m128 origz4;
			__m128 invdirx4;
			__m128 invdiry4;
			__m128 invdirz4;
		};

		struct QuadNode
		{
			union { __m128 minx4; float minx[4]; };
			union { __m128 miny4; float miny[4]; };
			union { __m128 minz4; float minz[4]; };
			union { __m128 maxx4; float maxx[4]; };
			union { __m128 maxy4; float maxy[4]; };
			union { __m128 maxz4; float maxz[4]; };
			int child[4];
			bool isLeaf[4];
			__m128 intersect(RaySIMD& r, __m128& dist) const;
		};

		AABB getSurroundAABB(Primitive** primitives, size_t size) const;
		size_t findSplit(Primitive** primitives, size_t size);
		::uint64_t expandBits(::uint64_t v) const;
		::uint64_t CalcMortonCode(glm::vec3& pt, glm::vec3& min, glm::vec3& max) const;
		void sortPrimitivesByMortonCodes();
		bool Traverse(Ray& ray, RaySIMD& rsimd, Intersection& intersect, QuadNode *node, int minLength);
		void buildQuadTree(QuadNode* parent, Node **children);
		void formQuadNode(Node *parent, Node **children);

		struct NodePair
		{
			int nodeNum;
			NodePair* closestNode;
			float dist;
		};

		int BuildTreeAgglomerative(NodePair *nodesArr, int size);
		void combineClusters(NodePair *nodesArr, int size,
			int amount);
		void findBestMatch(NodePair* node, NodePair *nodesArr, int count);
		int calcClusterSize(int amountOfNodes) const;


		std::vector<Primitive *> m_primitives;
		std::vector<Node> m_nodes;
		std::vector<QuadNode> m_quadNodes;
		int m_quadNodesCount = 0;
		int m_nodesCount = 0;

		static const size_t TREELET_SIZE = 7;
		static const size_t CLUSTER_SIZE = 20;
		const float CLUSTERFUNC_EPSILON = 0.1f;
	};
}
