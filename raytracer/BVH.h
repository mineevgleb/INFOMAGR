#pragma once
#include <CL/cl.hpp>
#include <vector>
#include <list>
#include "renederables/Primitive.h"
#include "AABB.h"
#include "gpu/opencl_structs.h"

namespace AGR
{
	class BVH
	{
	public:
		BVH(cl::Context *context, cl::Device *deviceToUse);
		~BVH();
		void constructLinear(std::vector<Primitive *>& primitives);
		void constructAgglomerative(std::vector<Primitive *>& primitives);
		bool Traverse(const Ray& ray, Intersection& intersect);
		void PacketTraverse(std::vector<Ray>& rays, std::vector<Intersection>& intersect);
		void PacketCheckOcclusions(std::vector<Ray>& rays, 
			std::vector<float>& lengths, std::vector<bool>& occlusionFlags);
		void PacketTraverseSlow(std::vector<Ray>& rays, std::vector<Intersection>& intersect);
	private:
		struct Node
		{
			AABB bounds;
			int leftFirst;
			int right;
			int count;
		};
		struct ConstructNode
		{
			AABB bounds;
			int leftFirst;
			int count;
			float SAH;
			float SAHcollapsed;
			bool isLeaf;
			int index;
		};
		AABB getSurroundAABB(Primitive** primitives, size_t size) const;
		size_t findSplit(Primitive** primitives, size_t size);
		void Subdivide(int nodeNum, Primitive** primitives, int first);
		void calcAABB(int nodeNum);
		bool collapseSomeNodes(int nodeNum);
		int findFirstIndex(int nodeNum);
		::uint64_t expandBits(::uint64_t v) const;
		::uint64_t CalcMortonCode(glm::vec3& pt, glm::vec3& min, glm::vec3& max) const;
		void sortPrimitivesByMortonCodes();
		float calcSAHvalue(int nodeNum);
		float calcSAHvalueCollapsed(int nodeNum);
		bool Traverse(const Ray& ray, Intersection& intersect, int nodeNum, glm::vec3& invRayDir);
		void composeTreelet(int nodeNum, int* leafs, int* childpairs);
		void reshapeTreelet(int nodeNum, const int* leafs, const int* childpairs);
		const int* executeReshape(::uint32_t curPartition, const ::uint32_t *bestPartitions, 
			const Node *leafs, const int *childpairs, const AABB* bounds, const int* primitivescounts,
			const bool* leafFlags);
		void improveBVH(int nodeNum);
		void fillGPUNodes();

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
		int calcClusterSize(int amountOfNodes);


		std::vector<Primitive *> m_primitives;
		std::vector<Node> m_nodes;
		std::vector<bool> m_leafFlags;
		std::vector<float> m_SAHcache;
		int m_nodesCount;
		std::vector<std::vector<unsigned int>> m_setsDescriptions;

		int depth = 0;
		int maxdepth = 0;

		std::vector<BVHnode_CL> m_gpuNodes;
		std::vector<Ray_CL> m_raysBuf;
		std::vector<Hit_CL> m_hitsBuf;
		std::vector<IncompleteTraversal_CL> m_restore;

		cl::Program *m_bvhProgram;
		cl::Context *m_context;
		cl::CommandQueue *m_cmdqueue;
		cl::Buffer *m_bvhBufCL = nullptr;
		cl::Buffer *m_raysBufCL = nullptr;
		cl::Buffer *m_hitsBufCL = nullptr;
		cl::Buffer *m_restoreBufCL = nullptr;
		cl::Buffer *m_hitsAmBufCL = nullptr;
		size_t m_bvhbufSize = 0;
		size_t m_raysBufCLSize = 0;

		static const size_t TREELET_SIZE = 7;
		static const size_t CLUSTER_SIZE = 20;
		const float CLUSTERFUNC_EPSILON = 0.1f;
	};
}
