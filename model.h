#pragma once

#include "common.h"

class Model
{
public:
	std::string		name;
	uint32_t		vb;
	uint32_t		ib;
	uint32_t		vbOffset;
	uint32_t		ibOffset;
	uint32_t		vbEnd;
	uint32_t		ibEnd;
	material_t		material;
};


class ModelInstance
{
public:
	std::vector<Triangle>	triList;
	AABB					aabb;
	uint32_t				vb;
	uint32_t				modelIx;
	mat4x4d					transform;
	vec3d					centroid;
	material_t				material;

	void ComputeAABB()
	{
		const size_t triCnt = triList.size();
		for ( size_t i = 0; i < triCnt; ++i )
		{
			aabb.Expand( triList[ i ].aabb.min );
			aabb.Expand( triList[ i ].aabb.max );
		}
	}
};

