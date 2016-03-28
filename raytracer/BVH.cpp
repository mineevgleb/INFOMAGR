#include "BVH.h"
#include "util.h"
#include <fstream>
#include <ppl.h>
#include <complex>

namespace AGR
{
	BVH::BVH(cl::Context* context, cl::Device* deviceToUse) 
		: m_nodesCount(0), m_context(context)
	{
		for (int i = 2; i <= TREELET_SIZE; ++i) {
			m_setsDescriptions.push_back(std::vector<unsigned int>());
			for (unsigned int j = 1; j < constpow(2, TREELET_SIZE); ++j) {
				if (setbitcnt(j) == i) {
					m_setsDescriptions[i - 2].push_back(j);
				}
			}
		}
		std::ifstream kernelSource("bvh_traversal.cl");
		std::string kernelString((std::istreambuf_iterator<char>(kernelSource)),
			(std::istreambuf_iterator<char>()));
		cl::Program::Sources sources;
		sources.push_back({ kernelString.c_str(), kernelString.length() });
		m_bvhProgram = new cl::Program(*context, sources);
		if (m_bvhProgram->build({ *deviceToUse }) != CL_SUCCESS) {
			std::ofstream errfile("builderr.txt");
			errfile << m_bvhProgram->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*deviceToUse) << '\n';
			errfile.close();
			throw std::runtime_error("Kernel build error");
		}
		m_cmdqueue = new cl::CommandQueue(*m_context, *deviceToUse);
		m_hitsAmBufCL = new cl::Buffer(*m_context, CL_MEM_READ_WRITE, sizeof(cl_int));
	}

	BVH::~BVH()
	{
		if (m_bvhBufCL) delete m_bvhBufCL;
		if (m_hitsBufCL) delete m_hitsBufCL;
		if (m_raysBufCL) delete m_raysBufCL;
		delete m_cmdqueue;
		delete m_bvhProgram;
	}

	void BVH::constructLinear(std::vector<Primitive*>& primitives)
	{
		m_primitives = primitives;
		sortPrimitivesByMortonCodes();
		m_nodesCount = 1;
		m_nodes.resize(primitives.size() * 2 - 1);
		m_leafFlags.resize(primitives.size() * 2 - 1);
		m_nodes[0].count = static_cast<int>(primitives.size());
		Subdivide(0, &primitives[0], 0);
		calcAABB(0);
		//improveBVH(0);
		collapseSomeNodes(0);
		fillGPUNodes();
	}

	void BVH::constructAgglomerative(std::vector<Primitive*>& primitives)
	{
		m_primitives = primitives;
		sortPrimitivesByMortonCodes();
		m_nodes.resize(m_primitives.size() * 2);
		m_leafFlags.resize(m_primitives.size() * 2);
		m_nodesCount = 1;
		std::vector<NodePair> nodes(m_primitives.size());
		for (int i = 0; i < m_primitives.size(); ++i) {
			nodes[i] = { i + 1, nullptr, FLT_MAX };
			m_nodes[m_nodesCount].leftFirst = i;
			m_nodes[m_nodesCount].bounds = m_primitives[i]->getBoundingBox();
			m_nodes[m_nodesCount].count = 1;
			m_leafFlags[m_nodesCount++] = true;
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
		collapseSomeNodes(0);
		fillGPUNodes();
	}

	bool BVH::Traverse(Ray& ray, Intersection& intersect)
	{
		ray.invDirection = 1.0f / ray.direction;
		float dist;
		if (!m_nodes[0].bounds.intersect(ray, dist)) {
			return false;
		}
		if(Traverse(ray, intersect, 0)) {
			return true;
		}
		return false;
	}

	void BVH::PacketTraverse(std::vector<Ray>& rays, std::vector<Intersection>& intersect)
	{
		intersect.resize(rays.size());
		std::vector<IntersectRequest_CL> reqests(rays.size());
		std::vector<Intersection_CL> intersectResults(rays.size());
		for (int i = 0; i < rays.size(); ++i) {
			reqests[i] = { i, 0 };
			rays[i].invDirection = 1.0f / rays[i].direction;
			intersect[i].ray_length = -1;
		}
		while (reqests.size() > 0) {
			calcIntercestionsCPU(rays, reqests, intersectResults);
			//for (int i = 0; i < reqests.size(); ++i) {
			concurrency::parallel_for(0, int(reqests.size()), 1, 
				[this, &reqests, &intersect, &rays, &intersectResults](int i) {
				Intersection& curIntersect = intersect[reqests[i].rayNum];
				bool goUp = true;
				if (intersectResults[i].flag & 1 && 
					(curIntersect.ray_length < 0 || curIntersect.ray_length > intersectResults[i].len)) {
					if (m_leafFlags[reqests[i].nodeNum]) {
						intersectWithPrimitives(reqests[i].nodeNum, rays[reqests[i].rayNum], curIntersect);
					} else {
						reqests[i].nodeNum = m_nodes[reqests[i].nodeNum].leftFirst;
						goUp = false;
					}
				}
				if (goUp) {
					while (true) {
						int parentNum = m_nodes[reqests[i].nodeNum].parent;
						if (parentNum == -1) {
							reqests[i].nodeNum = -1;
							break;
						}
						if (m_nodes[parentNum].right != reqests[i].nodeNum) {
							reqests[i].nodeNum = m_nodes[parentNum].right;
							break;
						}
						reqests[i].nodeNum = parentNum;
					}
				}
			});
			int curPos = 0;
			for (int i = 0; i < reqests.size(); ++i) {
				if (reqests[i].nodeNum > 0) {
					if (curPos != i) {
						reqests[curPos] = reqests[i];
					}
					++curPos;
				}
			}
			reqests.resize(curPos);
		}
		//const size_t GPU_EDGE = 4096 * 4;
		//if (rays.size() > GPU_EDGE) {
		//	
		//}
	}

	void BVH::PacketCheckOcclusions(std::vector<Ray>& rays, std::vector<float>& lengths,
		std::vector<bool>& occlusionFlags)
	{
		/*occlusionFlags.resize(rays.size());
		if (m_raysBuf.size() < rays.size()) {
			m_raysBuf.resize(rays.size());
			m_hitsBuf.resize(rays.size() * 32);
			m_restore.resize(rays.size());
		}
		if (m_raysBufCLSize < rays.size()) {
			if (m_raysBufCL) delete m_raysBufCL;
			if (m_restoreBufCL) delete m_restoreBufCL;
			if (m_hitsBufCL) delete m_hitsBufCL;
			m_raysBufCLSize = rays.size();
			m_raysBufCL = new cl::Buffer(*m_context, CL_MEM_READ_ONLY,
				m_raysBufCLSize * sizeof(m_raysBuf[0]));
			m_hitsBufCL = new cl::Buffer(*m_context, CL_MEM_READ_WRITE,
				m_raysBufCLSize * sizeof(m_hitsBuf[0]) * 32);
			m_restoreBufCL = new cl::Buffer(*m_context, CL_MEM_READ_WRITE,
				m_raysBufCLSize * sizeof(m_restoreBufCL[0]));
		}
		for (int i = 0; i < rays.size(); ++i) {
			m_raysBuf[i].origin = { rays[i].origin.x, rays[i].origin.y, rays[i].origin.z };
			glm::vec3 dir = rays[i].direction + glm::vec3(FLT_EPSILON);
			m_raysBuf[i].dir = { dir.x, dir.y, dir.z };
			m_raysBuf[i].inv_dir = { 1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z };
			occlusionFlags[i] = false;
		}
		memset(&m_restore[0], 0, sizeof(m_restore[0]) *  rays.size());
		m_cmdqueue->enqueueWriteBuffer(*m_raysBufCL, CL_TRUE, 0,
			rays.size() * sizeof(m_raysBuf[0]), &m_raysBuf[0]);
		int amountUndone = rays.size();
		cl_int zero = 0;
		cl_int hitsAm;
		while (amountUndone > 0) {
			m_cmdqueue->enqueueWriteBuffer(*m_restoreBufCL, CL_TRUE, 0,
				amountUndone * sizeof(m_restore[0]), &m_restore[0]);
			m_cmdqueue->enqueueWriteBuffer(*m_hitsAmBufCL, CL_TRUE, 0,
				sizeof(cl_int), &zero);
			cl::KernelFunctor traverseFunc(cl::Kernel(*m_bvhProgram, "traverseBVH"),
				*m_cmdqueue, cl::NullRange, cl::NDRange(amountUndone), cl::NullRange);
			traverseFunc(*m_raysBufCL, *m_bvhBufCL, *m_hitsBufCL, *m_restoreBufCL, *m_hitsAmBufCL, 
				static_cast<cl_int>(m_raysBufCLSize) * 32);
			m_cmdqueue->enqueueReadBuffer(*m_hitsAmBufCL, CL_TRUE, 0,
				sizeof(cl_int), &hitsAm);
			m_cmdqueue->enqueueReadBuffer(*m_hitsBufCL, CL_TRUE, 0,
				hitsAm * sizeof(m_hitsBuf[0]), &m_hitsBuf[0]);
			m_cmdqueue->enqueueReadBuffer(*m_restoreBufCL, CL_TRUE, 0,
				amountUndone * sizeof(m_restore[0]), &m_restore[0]);
			std::atomic<int> newAmountUndone = 0;
			//concurrency::parallel_for(0, hitsAm, 1,
			//[this, &rays, &intersect, &hitsAm](int i) {
			for (int i = 0; i < hitsAm; ++i) {
				Intersection curIntersect;
				int rayNum = m_hitsBuf[i].rayNum;
				if (occlusionFlags[rayNum]) continue;
				curIntersect.ray = rays[rayNum];
				size_t idx = m_gpuNodes[m_hitsBuf[i].nodeNum].leftFirst;
				for (int k = 0; k < m_gpuNodes[m_hitsBuf[i].nodeNum].count; ++k) {
					if (m_primitives[idx + k]->intersect(curIntersect)) {
						if (curIntersect.ray_length < lengths[rayNum]) {
							occlusionFlags[rayNum] = true;
							break;
						}
					}
				}
			}//);
			for (int i = 0; i < amountUndone; ++i) {
				if (m_restore[i].lastVisitedNode != -1 && !occlusionFlags[m_restore[i].rayNum]) {
					m_restore[newAmountUndone++] = m_restore[i];
				}
			}
			amountUndone = newAmountUndone;
		}*/
	}

	void BVH::PacketTraverseSlow(std::vector<Ray>& rays, std::vector<Intersection>& intersect)
	{
		intersect.resize(rays.size());
		concurrency::parallel_for(0, (int)rays.size(), 1,
			[this, &intersect, &rays](int i) {
			intersect[i].ray_length = -1;
			Traverse(rays[i], intersect[i]);
		});
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

	void BVH::Subdivide(int nodeNum, Primitive** primitives, int first)
	{
		if (m_nodes[nodeNum].count == 1) {
			m_nodes[nodeNum].leftFirst = first;
			m_leafFlags[nodeNum] = true;
			return;
		}
		m_leafFlags[nodeNum] = false;
		m_nodes[nodeNum].leftFirst = m_nodesCount;
		m_nodes[nodeNum].right = m_nodesCount + 1;
		size_t split = findSplit(primitives, m_nodes[nodeNum].count);
		m_nodesCount += 2;
		m_nodes[m_nodes[nodeNum].leftFirst].count = split;
		m_nodes[m_nodes[nodeNum].right].count = m_nodes[nodeNum].count - split;
		Subdivide(m_nodes[nodeNum].leftFirst, primitives, first);
		Subdivide(m_nodes[nodeNum].right, &primitives[split], first + split);
	}

	void BVH::calcAABB(int nodeNum)
	{
		if (m_nodes[nodeNum].count == 1) {
			m_nodes[nodeNum].bounds = m_primitives[m_nodes[nodeNum].leftFirst]->getBoundingBox();
			return;
		}
		calcAABB(m_nodes[nodeNum].leftFirst);
		calcAABB(m_nodes[nodeNum].leftFirst + 1);
		m_nodes[nodeNum].bounds = m_nodes[m_nodes[nodeNum].leftFirst].bounds;
		m_nodes[nodeNum].bounds.extend(m_nodes[m_nodes[nodeNum].leftFirst + 1].bounds);
	}

	bool BVH::collapseSomeNodes(int nodeNum)
	{
		if (m_leafFlags[nodeNum]) return true;
		if (collapseSomeNodes(m_nodes[nodeNum].leftFirst) &&
			collapseSomeNodes(m_nodes[nodeNum].leftFirst + 1) && 
			calcSAHvalueCollapsed(nodeNum) > calcSAHvalue(nodeNum)) {
			m_leafFlags[nodeNum] = true;
			return true;
		}
		return false;
	}

	int BVH::findFirstIndex(int nodeNum)
	{
		if (m_leafFlags[nodeNum]) {
			return m_nodes[nodeNum].leftFirst;
		}
		int left = findFirstIndex(m_nodes[nodeNum].leftFirst);
		int right = findFirstIndex(m_nodes[nodeNum].right);
		return left + (right - left) * (right < left);
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

	float BVH::calcSAHvalue(int nodeNum)
	{
		if (m_leafFlags[nodeNum]) {
			return m_nodes[nodeNum].bounds.calcArea()
				* m_nodes[nodeNum].count;
		}
		return 1.2f * m_nodes[nodeNum].bounds.calcArea() +
			calcSAHvalue(m_nodes[nodeNum].leftFirst) +
			calcSAHvalue(m_nodes[nodeNum].right);
	}

	float BVH::calcSAHvalueCollapsed(int nodeNum)
	{
		return m_nodes[nodeNum].bounds.calcArea()
			* m_nodes[nodeNum].count;
	}

	void BVH::calcIntercestionsCPU(std::vector<Ray>& rays, 
		std::vector<IntersectRequest_CL>& requests, std::vector<Intersection_CL>& results)
	{
		concurrency::parallel_for(0, int(requests.size()), 1, [this, &rays, &requests, &results](int i)
		{
			bool isHit = m_nodes[requests[i].nodeNum].bounds.intersect(
				rays[requests[i].rayNum], results[i].len);
			results[i].flag &= 0xFFFFFFFE;
			results[i].flag |= isHit;
		});
	}



	bool BVH::Traverse(Ray& ray, Intersection& intersect, int nodeNum)
	{
		if (m_leafFlags[nodeNum]) {
			if (m_nodes[nodeNum].count > 1) {
				bool wasHit = Traverse(ray, intersect, m_nodes[nodeNum].leftFirst);
				wasHit |= Traverse(ray, intersect, m_nodes[nodeNum].right);
				return wasHit;
			}
			if (m_nodes[nodeNum].count == 1) {
				Primitive* obj = m_primitives[m_nodes[nodeNum].leftFirst];
				float dist = obj->intersect(ray);
				if (dist > 0) {
					if (intersect.ray_length < 0 || dist < intersect.ray_length) {
						intersect.ray_length = dist;
						intersect.p_object = obj;
						return true;
					}
				}
				return false;
			}
		}
		int numLeft = m_nodes[nodeNum].leftFirst;
		int numRight = m_nodes[nodeNum].right;
		float distLeft = 0;
		float distRight = 0;
		bool intersLeft = m_nodes[numLeft].bounds.intersect(ray, distLeft);
		bool intersRight = m_nodes[numRight].bounds.intersect(ray, distRight);
		bool wasHit = false;
		if (distLeft < distRight) {
			if (intersLeft && (distLeft < intersect.ray_length || intersect.ray_length < 0))
				wasHit |= Traverse(ray, intersect, numLeft);
			if (intersRight && (distRight < intersect.ray_length || intersect.ray_length < 0))
				wasHit |= Traverse(ray, intersect, numRight);
		} else {
			if (intersRight && (distRight < intersect.ray_length || intersect.ray_length < 0))
				wasHit |= Traverse(ray, intersect, numRight);
			if (intersLeft && (distLeft < intersect.ray_length || intersect.ray_length < 0))
				wasHit |= Traverse(ray, intersect, numLeft);
		}
		return wasHit;
	}

	void BVH::intersectWithPrimitives(int nodeNum, Ray &r, Intersection& hit)
	{
		if (m_nodes[nodeNum].count > 1) {
			intersectWithPrimitives(m_nodes[nodeNum].leftFirst, r, hit);
			intersectWithPrimitives(m_nodes[nodeNum].right, r, hit);
		} else {
			Primitive* obj = m_primitives[m_nodes[nodeNum].leftFirst];
			float dist = obj->intersect(r);
			if (dist > 0 && (dist < hit.ray_length || hit.ray_length < 0)) {
				hit.ray_length = dist;
				hit.p_object = obj;
			}
		}
	}

	void BVH::composeTreelet(int nodeNum, int* leafs, int* childpairs)
	{
		if (m_nodes[nodeNum].count < TREELET_SIZE) return;
		int leafsCount = 2;
		leafs[0] = m_nodes[nodeNum].leftFirst;
		leafs[1] = m_nodes[nodeNum].right;
		childpairs[0] = m_nodes[nodeNum].leftFirst;
		float areas[7];
		areas[0] = m_nodes[leafs[0]].bounds.calcArea();
		areas[1] = m_nodes[leafs[1]].bounds.calcArea();
		while (leafsCount < TREELET_SIZE) {
			float maxArea = -FLT_MAX;
			int nodeToExpand = -1;
			for (int i = 0; i < leafsCount; ++i) {
				bool cmp = areas[i] > maxArea && !m_leafFlags[leafs[i]];
				maxArea += (areas[i] - maxArea) * cmp;
				nodeToExpand += (i - nodeToExpand) * cmp;
			}
			childpairs[leafsCount - 1] = m_nodes[leafs[nodeToExpand]].leftFirst;
			leafs[leafsCount] = m_nodes[leafs[nodeToExpand]].right;
			leafs[nodeToExpand] = m_nodes[leafs[nodeToExpand]].leftFirst;
			areas[nodeToExpand] = m_nodes[leafs[nodeToExpand]].bounds.calcArea();
			areas[leafsCount] = m_nodes[leafs[leafsCount]].bounds.calcArea();
			leafsCount++;
		}
	}

	void BVH::reshapeTreelet(int nodeNum, const int* leafs, const int* childpairs)
	{
		const int setAm = constpow(2, TREELET_SIZE) - 1;
		float areas[setAm];
		AABB boxes[setAm];
		float SAHvalues[setAm];
		::uint32_t bestPartitions[setAm];
		int primitivesCount[setAm];
		//init arrays
		for (int i = 0; i < setAm; ++i) {
			boxes[i] = AABB(glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX));
			primitivesCount[i] = 0;
		}
		// calculate area for each set of leaves
		int step = 1;
		for (int i = 0; i < TREELET_SIZE; ++i) {
			int pos = setAm - 1;
			while(pos >= 0) {
				for (int j = 0; j < step && pos >= 0; ++j) {
						boxes[pos--].extend(m_nodes[leafs[i]].bounds);
				}
				pos -= step;
			}
			step *= 2;
		}
		for (int i = 0; i < setAm; ++i) {
			areas[i] = boxes[i].calcArea();
		}
		int cur = 1;
		//calculate optimal tree
		for (int i = 0; i < TREELET_SIZE; ++i) {
			SAHvalues[cur - 1] = calcSAHvalue(leafs[i]);
			primitivesCount[cur - 1] = m_nodes[leafs[i]].count;
			cur *= 2;
		}
		for (int k = 2; k <= TREELET_SIZE; ++k) {
			for (unsigned int set : m_setsDescriptions[k - 2]) {
				unsigned delta = (set - 1) & set;
				unsigned p = (-delta) & set;
				float bestSAH = FLT_MAX;
				primitivesCount[set - 1] = primitivesCount[p - 1] + 
					primitivesCount[(set ^ p) - 1];
				do {
					float curSAH = SAHvalues[p - 1] + SAHvalues[(set ^ p) - 1];
					if (curSAH < bestSAH) {
						bestSAH = curSAH;
						bestPartitions[set - 1] = p;
					}
					p = (p - delta) & set;
				} while (p != 0);
				float SAHuncollapsed = 1.2 * areas[set - 1] + bestSAH;
				float SAHcollapsed = areas[set - 1] * primitivesCount[set - 1];
				bool cmp = SAHcollapsed < SAHuncollapsed;
				SAHvalues[set - 1] = SAHuncollapsed + (SAHcollapsed - SAHuncollapsed) * cmp;
			}
		}
		Node leafnodes[TREELET_SIZE];
		bool leafflags[TREELET_SIZE];
		for (int i = 0; i < TREELET_SIZE; ++i) {
			leafnodes[i] = m_nodes[leafs[i]];
			leafflags[i] = m_leafFlags[leafs[i]];
		}
		m_nodes[nodeNum].leftFirst = childpairs[0];
		m_nodes[nodeNum].right = childpairs[0] + 1;
		
		executeReshape(setAm, bestPartitions, leafnodes, 
			&childpairs[0], boxes, primitivesCount, leafflags);

	}

	const int* BVH::executeReshape(::uint32_t curPartition, const ::uint32_t* bestPartitions, 
		const Node* leafs, const int* childpairs, const AABB* bounds, const int* primitivescounts,
		const bool* leafFlags)
	{
		::uint32_t partition[2];
		partition[0] = bestPartitions[curPartition - 1];
		partition[1] = curPartition ^ bestPartitions[curPartition - 1];
		const int* actualChildpairs = &childpairs[1];
		for (int i = 0; i < 2; ++i) {
			if (setbitcnt(partition[i]) == 1) {
				size_t index = 31 - lzcnt32(partition[i]);
				m_nodes[childpairs[0] + i] = leafs[index];
				m_leafFlags[childpairs[0] + i] = leafFlags[index];
			} else {
				m_nodes[childpairs[0] + i].leftFirst = actualChildpairs[0];
				m_nodes[childpairs[0] + i].right = actualChildpairs[0] + 1;
				m_nodes[childpairs[0] + i].bounds = bounds[partition[i] - 1];
				m_nodes[childpairs[0] + i].count = primitivescounts[partition[i] - 1];
				m_leafFlags[childpairs[0] + i] = false;
				actualChildpairs = executeReshape(partition[i], bestPartitions, leafs, actualChildpairs,
					bounds, primitivescounts, leafFlags);
			}
		}
		return actualChildpairs;
	}

	void BVH::improveBVH(int nodeNum)
	{
		if (m_nodes[nodeNum].count < TREELET_SIZE) return;
		improveBVH(m_nodes[nodeNum].leftFirst);
		improveBVH(m_nodes[nodeNum].right);
		int leafs[TREELET_SIZE];
		int childpairs[TREELET_SIZE - 1];
		composeTreelet(nodeNum, leafs, childpairs);
		reshapeTreelet(nodeNum, leafs, childpairs);
	}

	void BVH::fillGPUNodes()
	{
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
		int firstPrimitiveIdx = m_nodes[nodesArr->nodeNum].leftFirst;
		volatile int asd = 0;
		if (firstPrimitiveIdx == 155)
			++asd;
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
			m_nodes[m_nodesCount].leftFirst = bestPair->nodeNum;
			m_nodes[m_nodesCount].right = bestPair->closestNode->nodeNum;
			m_nodes[m_nodesCount].bounds = m_nodes[bestPair->nodeNum].bounds;
			m_nodes[m_nodesCount].bounds.extend(m_nodes[bestPair->closestNode->nodeNum].bounds);
			m_nodes[m_nodesCount].count = m_nodes[m_nodes[m_nodesCount].leftFirst].count;
			m_nodes[m_nodesCount].count += m_nodes[m_nodes[m_nodesCount].right].count;
			m_nodes[m_nodesCount].parent = -1;
			m_nodes[bestPair->nodeNum].parent = m_nodesCount;
			m_nodes[bestPair->closestNode->nodeNum].parent = m_nodesCount;
			m_leafFlags[m_nodesCount] = false;
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
