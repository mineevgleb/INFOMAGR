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
	cl_int parent;
	cl_int align;
};

struct Ray_CL
{
	cl_float3 origin;
	cl_float3 dir;
	cl_float3 inv_dir;
};

struct HitRecord_CL
{
	cl_int rayNum;
	cl_int leafsHits[5];
	cl_int lastNode;
	cl_int hitsFound;
};