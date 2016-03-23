#include "BVH.h"
#include "util.h"
#include <fstream>
#include <ppl.h>
#include <complex>


namespace AGR
{
	BVH::BVH(cl::Context* context, cl::Device* deviceToUse) 
		: m_context(context)
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
		m_nodes[0].count = primitives.size();
		Subdivide(0, &primitives[0], 0);
		calcAABB(0);
		//improveBVH(0);
		collapseSomeNodes(0);
		fillGPUNodes();
	}

	void BVH::constructAgglomerative(std::vector<Primitive*>& primitives)
	{
		m_primitives = primitives;
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
		int newSize = BuildTreeAgglomerative(&nodes[0], m_primitives.size());
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
		fillGPUNodes();
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

	void BVH::PacketTraverse(std::vector<Ray>& rays, std::vector<Intersection>& intersect)
	{
		intersect.resize(rays.size());
		static std::vector<int> rayStartsInArray;
		if (m_raysBuf.size() < rays.size()) {
			m_raysBuf.resize(rays.size());
			m_hitsBuf.resize(rays.size() * 32);
			m_restore.resize(rays.size());
			rayStartsInArray.resize(rays.size());
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
			m_raysBuf[i].dir = {dir.x, dir.y, dir.z};
			m_raysBuf[i].inv_dir = { 1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z };
			intersect[i].ray_length = -1;
			intersect[i].ray = rays[i];
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
			concurrency::parallel_buffered_sort(m_hitsBuf.begin(), m_hitsBuf.begin() + hitsAm,
				[](Hit_CL& a, Hit_CL& b)->bool {return a.rayNum != b.rayNum ? 
			a.rayNum < b.rayNum : a.len < b.len;});
			int curPos = 0;
			for (int i = 0; i < hitsAm; ++i) {
				if (i == 0 || m_hitsBuf[i].rayNum != m_hitsBuf[i - 1].rayNum) {
					rayStartsInArray[curPos++] = i;
				}
			}
			concurrency::parallel_for(0, amountUndone, 1,
			[this, &rays, &intersect, &hitsAm](int i) {
			//for (int i = 0; i < amountUndone; ++i){
				Intersection curIntersect;
				int beg = rayStartsInArray[i];
				int rayNum = m_hitsBuf[beg].rayNum;
				curIntersect.ray = rays[rayNum];
				AABB firstAABB = m_nodes[0].bounds;
				for (int j = beg; m_hitsBuf[j].rayNum == rayNum && j < hitsAm; ++j) {
					if (!firstAABB.testOverlap(m_nodes[m_hitsBuf[j].nodeNum].bounds)) break;
					size_t idx = m_gpuNodes[m_hitsBuf[j].nodeNum].leftFirst;
					for (int k = 0; k < m_gpuNodes[m_hitsBuf[j].nodeNum].count; ++k) {
						if (m_primitives[idx + k]->intersect(curIntersect)) {
							if (curIntersect.ray_length < intersect[rayNum].ray_length
								|| intersect[rayNum].ray_length < 0) {
								firstAABB = curIntersect.p_object->getBoundingBox();
								intersect[rayNum] = curIntersect;
							}
						}
					}
				}
			});
			for (int i = 0; i < amountUndone; ++i) {
				if (m_restore[i].lastVisitedNode != -1) {
					int idx = newAmountUndone++;
					m_restore[idx] = m_restore[i];
				}
			}
			amountUndone = newAmountUndone;
		}
	}

	void BVH::PacketCheckOcclusions(std::vector<Ray>& rays, std::vector<float>& lengths,
		std::vector<bool>& occlusionFlags)
	{
		occlusionFlags.resize(rays.size());
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
		}
	}

	void BVH::PacketTraverseSlow(std::vector<Ray>& rays, std::vector<Intersection>& intersect)
	{
		intersect.resize(rays.size());
		for (int i = 0; i < rays.size(); ++i) {
			intersect[i].ray = rays[i];
			intersect[i].ray_length = -1;
			Traverse(rays[i], intersect[i]);
		}
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
		depth++;
		if (depth > maxdepth) maxdepth = depth;
		if (m_nodes[nodeNum].count == 1) {
			m_nodes[nodeNum].leftFirst = first;
			m_leafFlags[nodeNum] = true;
			depth--;
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
		depth--;
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
			m_nodes[nodeNum].leftFirst = findFirstIndex(nodeNum);
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

	bool BVH::Traverse(const Ray& ray, Intersection& intersect, int nodeNum, glm::vec3& invRayDir)
	{
		bool inters = m_nodes[nodeNum].bounds.testIntersection(ray, invRayDir);
		if (!inters) return false;
		if (m_nodes[nodeNum].count > 0) {
			Intersection test;
			bool wasHit = false;
			for (int i = 0; i < m_nodes[nodeNum].count; i++) {
				test.ray = ray;
				if (m_primitives[m_nodes[nodeNum].leftFirst + i]->intersect(test)) {
					if (intersect.ray_length < 0 || test.ray_length < intersect.ray_length) {
						intersect = test;
						wasHit = true;
					}
				}
			}
			return wasHit;
		}
		bool t1 = Traverse(ray, intersect, m_nodes[nodeNum].leftFirst, invRayDir);
		bool t2 = Traverse(ray, intersect, m_nodes[nodeNum].right, invRayDir);
		return t1 || t2;
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
		m_gpuNodes.resize(m_nodesCount);
		for (int i = 0; i < m_nodesCount; ++i) {
			if (!m_leafFlags[i])
			{
				m_nodes[i].count = -1;
				m_gpuNodes[m_nodes[i].leftFirst].parent = i;
				m_gpuNodes[m_nodes[i].right].parent = i;
			}
			m_gpuNodes[i].leftFirst = m_nodes[i].leftFirst;
			m_gpuNodes[i].right = m_nodes[i].right;
			m_gpuNodes[i].count = m_nodes[i].count;
			glm::vec3 minPt = m_nodes[i].bounds.getMinPt() + glm::vec3(FLT_EPSILON);
			glm::vec3 maxPt = m_nodes[i].bounds.getMaxPt() + glm::vec3(FLT_EPSILON);;
			m_gpuNodes[i].bounds.minpt = { minPt.x, minPt.y, minPt.z };
			m_gpuNodes[i].bounds.maxpt = { maxPt.x, maxPt.y, maxPt.z };
		}
		m_gpuNodes[0].parent = -1;
		if (m_bvhbufSize < m_nodesCount) {
			if (m_bvhBufCL) delete m_bvhBufCL;
			m_bvhbufSize = m_nodesCount * 2;
			m_bvhBufCL = new cl::Buffer(*m_context, CL_MEM_READ_ONLY,
				sizeof(m_gpuNodes[0]) * m_bvhbufSize);
		}
		m_cmdqueue->enqueueWriteBuffer(*m_bvhBufCL, CL_TRUE, 0,
			sizeof(m_gpuNodes[0]) * m_nodesCount, &m_gpuNodes[0]);
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
			m_leafFlags[m_nodesCount] = false;
			NodePair* deleted1 = bestPair->closestNode;
			NodePair* deleted2 = bestPair;
			*bestPair->closestNode = { -1, nullptr, FLT_MAX };
			*bestPair = { m_nodesCount, nullptr, 0.0f };
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

	int BVH::calcClusterSize(int amountOfNodes)
	{
		const float scaler = pow(CLUSTER_SIZE, 0.5f + CLUSTERFUNC_EPSILON) / 2;
		return round(scaler * pow(amountOfNodes, 0.5f - CLUSTERFUNC_EPSILON));
	}
}
