#pragma once
#include <CL\cl.hpp>

struct AABB_CL
{
	cl_float3 minpt;
	cl_float3 maxpt;
};

struct BVHnode_CL
{
	AABB_CL bounds;
	cl_int count;
	cl_int leftFirst;
	cl_int right;
	cl_int parent;
};

struct Ray_CL
{
	cl_float3 origin;
	cl_float3 dir;
	cl_float3 inv_dir;
};

#define HIT_BUF_SIZE 5
struct HitRecord_CL
{
	cl_int rayNum;
	cl_int leafsHits[HIT_BUF_SIZE];
	cl_float hitLengths[HIT_BUF_SIZE];
	cl_int lastNode;
	cl_int hitsFound;
};

struct Hit_CL
{
	cl_int rayNum;
	cl_int nodeNum;
	cl_float len;
};

struct IncompleteTraversal_CL
{
	cl_int rayNum;
	cl_int lastVisitedNode;
};