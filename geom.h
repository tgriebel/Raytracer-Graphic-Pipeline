#pragma once

#include <utility>
#include "../GfxCore/mathVector.h"
#include "../GfxCore/matrix.h"
#include "../GfxCore/color.h"
#include "../GfxCore/meshIO.h"
#include "../GfxCore/aabb.h"
#include "../GfxCore/ray.h"
#include "octree.h"
#include "common.h"

struct plane_t
{
	vec4d halfWidth;
	vec4d halfHeight;
	vec4d origin;
};


struct vertex_t
{
	vec4d	pos;
	vec3d	normal;
	vec3d	tangent;
	vec3d	bitangent;
	vec2d	uv;
	Color	color;
};


inline bool operator==( const vertex_t& vertex0, const vertex_t& vertex1 )
{
	return (	( vertex0.pos == vertex1.pos )
			&& 	( vertex0.normal == vertex1.normal )
			&&	( vertex0.uv == vertex1.uv )
			&&	( vertex0.color == vertex1.color ) );
}


struct edge_t
{
	vec4d	pt0;
	vec4d	pt1;
	double	length;
	AABB	aabb;
};


struct box_t
{
	vec4d	pts[ 8 ];
	double	width;
	double	height;
	double	depth;
	AABB	aabb;
};

struct vertexBuffer_t
{
	std::vector<vertex_t> buffer;
};


struct indexBuffer_t
{
	std::vector<uint32_t> buffer;
};

using imageHandle_t = uint32_t;

struct Triangle
{
	vertex_t	v0;
	vertex_t	v1;
	vertex_t	v2;

	vec3d		e0;
	vec3d		e1;
	vec3d		e2;

	vec3d		n;
	vec3d		t;
	vec3d		b;
	AABB		aabb;

	Triangle( const vertex_t& _v0, const vertex_t& _v1, const vertex_t& _v2 )
	{
		v0 = _v0;
		v1 = _v1;
		v2 = _v2;

		vec3d pt0 = Trunc<4, 1>( v0.pos );
		vec3d pt1 = Trunc<4, 1>( v1.pos );
		vec3d pt2 = Trunc<4, 1>( v2.pos );

		e0 = ( pt1 - pt0 );
		e1 = ( pt2 - pt1 );
		e2 = ( pt2 - pt0 );

		n = Cross( e0, e1 ).Normalize();

		t = e0.Normalize();
		b = Cross( n, t );

		aabb.Expand( pt0 );
		aabb.Expand( pt1 );
		aabb.Expand( pt2 );
	}
};


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
	std::vector<Triangle>	triCache;
	Octree<uint32_t>		octree;
	uint32_t				vb;
	uint32_t				modelIx;
	mat4x4d					transform;
	vec3d					centroid;
	material_t				material;

	void BuildAS()
	{
		AABB aabb;

		const size_t triCnt = triCache.size();
		for ( size_t i = 0; i < triCnt; ++i )
		{
			aabb.Expand( triCache[ i ].aabb.min );
			aabb.Expand( triCache[ i ].aabb.max );
		}

		octree = Octree<uint32_t>( aabb.min, aabb.max );

		for( uint32_t i = 0; i < triCnt; ++ i )
		{
			// Build the octree using triangle indices
			octree.Insert( triCache[ i ].aabb, i );
		}
	}
};


/*
===================================
PointToBarycentric
===================================
*/
inline vec3d PointToBarycentric( const vec3d& pt, const vec3d& v0, const vec3d& v1, const vec3d& v2 )
{
	vec3d e1 = v2 - v1;
	vec3d e2 = v0 - v2;
	vec3d e3 = v1 - v0;

	vec3d d1 = pt - v0;
	vec3d d2 = pt - v1;
	vec3d d3 = pt - v2;

	vec3d n = Cross( e1, e2 ) / ( Cross( e1, e2 ).Length() );

	const double area = Dot( Cross( e1, e2 ), n );

	vec3d baryPoint;
	baryPoint[ 0 ] = Dot( Cross( e1, d3 ), n ) / area;
	baryPoint[ 1 ] = Dot( Cross( e2, d1 ), n ) / area;
	baryPoint[ 2 ] = Dot( Cross( e3, d2 ), n ) / area;

	return baryPoint;
}


/*
===================================
RayToTriangleIntersection
- Möller–Trumbore ray-triangle intersection algorithm
===================================
*/
inline bool RayToTriangleIntersection( const Ray& r, const Triangle& tri, bool& outBackface, double& outT )
{
	const double	epsilon	= 1e-7;
	const vec3d		e0		= tri.e0;
	const vec3d		e1		= tri.e2;
	const vec3d		p		= Cross( r.d, e1 );
	const vec3d		h		= ( r.GetOrigin() - Trunc<4, 1>( tri.v0.pos ) );
	const vec3d		q		= Cross( h, e0 );
	const double	det		= Dot( e0, p );
	const double	invDet	= ( 1.0 / det );
	const double	u		= invDet * Dot( h, p );
	const double	v		= invDet * Dot( r.d, q );

	outT = 0.0;
	outBackface = true;

	if ( fabs( det ) < epsilon ) // Ray is parallel
	{
		return false;
	}

	if ( ( u < 0.0 ) || ( u > 1.0 ) )
	{
		return false;
	}

	if ( ( v < 0.0 ) || ( ( u + v ) > 1.0 ) )
	{
		return false;
	}

	const double t = Dot( e1, q ) * invDet;

	if ( r.Inside( t ) ) // Within ray parameterization
	{
		outT = t;
		outBackface = ( det < epsilon );
		return true;
	}
	else
	{
		return false;
	}
}


uint32_t LoadModel( const std::string& path, const uint32_t vb, const uint32_t ib );
uint32_t LoadModelObj( const std::string& path, const uint32_t vb, const uint32_t ib );
void StoreModelObj( const std::string& path, const uint32_t modelIx );
void CreateModelInstance( const uint32_t modelIx, const mat4x4d& modelMatrix, const bool smoothNormals, const Color& tint, ModelInstance* outInstance, const material_t* material = nullptr );
uint32_t CreatePlaneModel( const uint32_t vb, const uint32_t ib, const vec2d& size, const vec2i& cellCnt );