#pragma once

#include <utility>
#include "mathVector.h"
#include "matrix.h"
#include "color.h"

struct Ray
{
	Ray() : t( 0.0 ), mint( 1e-7 ), maxt( DBL_MAX ) {}

	Ray( const vec3d& origin, const vec3d& target, const double _minT = 0.0, const double _maxT = DBL_MAX )
	{
		o = origin;
		d = target - origin;
		t = -1.0;
		mint = std::max( 0.0, _minT );
		maxt = std::max( 0.0, _maxT );

		if ( mint > maxt )
		{
			std::swap( mint, maxt );
		}
	}

	void MarchTo( const double _t )
	{
		t = Clamp( _t, mint, maxt );
	}

	vec3d GetOrigin()
	{
		return o;
	}

	vec3d GetPoint()
	{
		return ( o + t * d );
	}

	vec3d GetEndPoint()
	{
		return ( o + maxt * d );
	}

	vec3d GetVector()
	{
		return d.Normalize();
	}

	vec3d d;
	vec3d o;
	double t;
	double mint;
	double maxt;
};


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
	vec2d	uv;
	Color	color;
};


struct AABB
{
	vec3d min;
	vec3d max;

	AABB()
	{
		min[ 0 ] = DBL_MAX;
		min[ 1 ] = DBL_MAX;
		min[ 2 ] = DBL_MAX;

		max[ 0 ] = -DBL_MAX;
		max[ 1 ] = -DBL_MAX;
		max[ 2 ] = -DBL_MAX;
	}


	AABB( const vec3d& min_i, const vec3d& max_i )
	{
		min = min_i;
		max = max_i;
	}

	// Adapts Christer Ericson's Kay-Kajiya slab based interection test from Real Time Collision Detection
	bool Intersect( const Ray& r, double& tnear, double& tfar ) const
	{
		double tmin = -DBL_MAX;
		double tmax = DBL_MAX;
		for ( uint32_t i = 0; i < 3; ++i )
		{
			if ( abs( r.d[ i ] ) < 0.000001 ) // Parallel to slab...
			{
				if ( ( r.o[ i ] < min[ i ] ) || ( r.o[ i ] > max[ i ] ) ) // ...and outside box
					return false;
			} 
			else
			{
				double ood = 1.0 / r.d[ i ];
				double t1 = ( min[ i ] - r.o[ i ] ) * ood;
				double t2 = ( max[ i ] - r.o[ i ] ) * ood;

				if( t1 > t2 )
					std::swap( t1, t2 );

				if ( t1 > tmin )
					tmin = t1;

				if ( t2 < tmax )
					tmax = t2;

				if ( ( tmin > tmax ) || ( t2 < 0 ) )
					return false;
			}
		}
		
		return true;
	}

	bool Inside( const vec3d& pt )
	{
		bool isInside = true;
		isInside = isInside && ( pt[ 0 ] >= min[ 0 ] && pt[ 0 ] <= max[ 0 ] );
		isInside = isInside && ( pt[ 1 ] >= min[ 1 ] && pt[ 1 ] <= max[ 1 ] );
		isInside = isInside && ( pt[ 2 ] >= min[ 2 ] && pt[ 2 ] <= max[ 2 ] );
		return isInside;
	}

	void Expand( const vec3d& pt )
	{
		min[ 0 ] = std::min( pt[ 0 ], min[ 0 ] );
		min[ 1 ] = std::min( pt[ 1 ], min[ 1 ] );
		min[ 2 ] = std::min( pt[ 2 ], min[ 2 ] );

		max[ 0 ] = std::max( pt[ 0 ], max[ 0 ] );
		max[ 1 ] = std::max( pt[ 1 ], max[ 1 ] );
		max[ 2 ] = std::max( pt[ 2 ], max[ 2 ] );
	}
};


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

		n = Cross( e0, e2 ).Normalize();

		t = e0.Normalize();
		b = Cross( n, t );

		aabb.Expand( pt0 );
		aabb.Expand( pt1 );
		aabb.Expand( pt2 );
	}
};


vec3d PointToBarycentric( const vec3d& pt, const vec3d& v0, const vec3d& v1, const vec3d& v2 )
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


// Möller–Trumbore ray-triangle intersection algorithm
bool RayToTriangleIntersection( Ray& r, const Triangle& tri, bool& outBackface, double& outT )
{
	const double	epsilon	= 1e-7;
	const vec3d		e0		= tri.e0;
	const vec3d		e1		= tri.e2;
	const vec3d		p		= Cross( r.d, e1 );
	const vec3d		h		= ( r.o - Trunc<4, 1>( tri.v0.pos ) );
	const vec3d		q		= Cross( h, e0 );
	const double	det		= Dot( e0, p );
	const double	invDet	= ( 1.0 / det );
	const double	u		= invDet * Dot( h, p );
	const double	v		= invDet * Dot( r.d, q );

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

	outT = Dot( e1, q ) * invDet;

	if ( ( outT >= r.mint ) && ( outT <= r.maxt ) ) // Within ray parameterization
	{
		outBackface = ( det < epsilon );
		//outBackface = Dot( r.d, tri.n ) >= 0.0; // winding order independent
		return true;
	}
	else
	{
		return false;
	}
}
