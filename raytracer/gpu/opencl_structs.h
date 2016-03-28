#pragma once
#include <CL\cl.hpp>

struct AABB_CL
{
	cl_float3 minpt;
	cl_float3 maxpt;
};

struct Ray_CL
{
	cl_float3 origin;
	cl_float3 dir;
	cl_float3 inv_dir;
};

struct IntersectRequest_CL
{
	int rayNum;
	int nodeNum;
};

union Intersection_CL
{
	unsigned int flag;
	float len;
};