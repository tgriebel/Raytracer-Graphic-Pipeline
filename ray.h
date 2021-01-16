#pragma once

#include <utility>
#include "../GfxCore/mathVector.h"
#include "../GfxCore/matrix.h"
#include "common.h"

struct Ray
{
	Ray() : mint( 1e7 ), maxt( DBL_MAX ) {}

	Ray( const vec3d& origin, const vec3d& target, const double _minT = 1e-7 )
	{
		o = origin;
		d = target - origin;
		t = d.Length();
		d = d.Normalize();

		mint = std::max( 0.0, _minT );
		maxt = std::max( 0.0, t - mint );
	}

	bool Inside( const double _t ) const
	{
		return ( _t >= mint ) && ( _t <= maxt );
	}

	vec3d GetPoint( const double _t ) const
	{
		double t = Clamp( _t, mint, maxt );
		return ( o + t * d );
	}

	vec3d GetOrigin() const
	{
		return o;
	}

	vec3d GetEndPoint() const
	{
		return ( o + d );
	}

	vec3d GetVector() const
	{
		return d.Normalize();
	}

	vec3d d;
	vec3d o;
	// private:
	double t;
	double mint;
	double maxt;
};
