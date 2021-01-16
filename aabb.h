#pragma once

#include <utility>
#include "../GfxCore/mathVector.h"
#include "../GfxCore/matrix.h"
#include "ray.h"

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

	AABB( const vec3d& point )
	{
		min = point;
		max = point;
	}

	AABB( const vec3d& _min, const vec3d& _max )
	{
		Expand( _min );
		Expand( _max );
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

				if ( t1 > t2 )
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

	bool Inside( const AABB& aabb )
	{
		return ( Inside( aabb.min ) && Inside( aabb.max ) );
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

	vec3d GetMin() const
	{
		return min;
	}

	vec3d GetMax() const
	{
		return max;
	}
};
