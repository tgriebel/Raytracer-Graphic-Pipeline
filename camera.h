#pragma once

#include "common.h"
#include "../GfxCore/mathVector.h"
#include "geom.h"

class Camera
{
public:
	Camera() {}

	Camera( const vec4d& _origin, const vec4d& _X, const vec4d& _Y, const vec4d& _Z, const double _fovDegrees, const double _aspect, const double _near, const double _far )
	{
		double aspectRatio = ( _aspect != 0.0 ) ? _aspect : 1.0;

		halfFovX = tan( 0.5 * Radians( _fovDegrees ) );
		halfFovY = tan( 0.5 * Radians( _fovDegrees ) ) / aspectRatio;
		near = _near;
		far = _far;
		aspect = aspectRatio;
		focalLength = far;
		viewportWidth = 2.0 * halfFovX;
		viewportHeight = 2.0 * halfFovY;
		origin = _origin;
		xAxis = _X;
		yAxis = _Y;
		zAxis = _Z;
	}

	/*
	Camera( const vec4d& _origin, const vec4d& _at, const vec4d& _up, const double _fovX, const double _aspect, const double _near, const double _far )
	{
		assert( false );
	}
	*/

	plane_t GetFocalPlane() const
	{
		plane_t plane;
		plane.origin = origin - focalLength * zAxis;
		plane.halfWidth = 0.5 * focalLength * viewportWidth * xAxis;
		plane.halfHeight = 0.5 * focalLength * viewportHeight * yAxis;
		return plane;
	}

	Ray GetViewRay( const vec2d& uv ) const
	{
		// TODO: clip by near plane
		plane_t plane = GetFocalPlane();
		vec4d corner = plane.origin - plane.halfWidth - plane.halfHeight;
		vec4d viewPt = corner + vec4d( 2.0 * uv[ 0 ] * plane.halfWidth + 2.0 * uv[ 1 ] * plane.halfHeight );

		Ray ray = Ray( Trunc<4, 1>( origin ), Trunc<4, 1>( viewPt ), 0.0 );

		return ray;
	}

	mat4x4d ToViewMatrix() const
	{
		return CreateMatrix4x4(	xAxis[ 0 ],	xAxis[ 1 ],	xAxis[ 2 ],	-Dot( xAxis, origin ),
								yAxis[ 0 ],	yAxis[ 1 ],	yAxis[ 2 ],	-Dot( yAxis, origin ),
								zAxis[ 0 ],	zAxis[ 1 ],	zAxis[ 2 ],	-Dot( zAxis, origin ),
								0.0,		0.0,		0.0,		1.0 );
	}

	mat4x4d ToOrthographicProjMatrix( double w, double h ) const
	{
		const double width = ( w != 0.0 ) ? w : 1.0;
		const double height = ( h != 0.0 ) ? h : 1.0;

		mat4x4d m;
		m[ 0 ][ 0 ] = 2.0 / width;
		m[ 0 ][ 3 ] = -1.0;
		m[ 1 ][ 1 ] = 2.0 / height;
		m[ 1 ][ 3 ] = -1.0;
		m[ 2 ][ 2 ] = -2.0 / ( far - near );
		m[ 2 ][ 3 ] = -( far + near ) / ( far - near );
		m[ 3 ][ 3 ] = 1.0;

		return m;
	}

	mat4x4d ToPerspectiveProjMatrix() const
	{
		mat4x4d m;
		m[ 0 ][ 0 ] = 1.0 / halfFovX;
		m[ 1 ][ 1 ] = 1.0 / halfFovY;
		m[ 2 ][ 2 ] = -far / ( far - near );
		m[ 2 ][ 3 ] = -( far * near ) / ( far - near );
		m[ 3 ][ 2 ] = -1.0;
		return m;
	}

	double aspect;
	double halfFovX;
	double halfFovY;
	double focalLength;
	double near;
	double far;
	double viewportWidth;
	double viewportHeight;

	vec4d localOrigin;
	vec4d origin;
	vec4d xAxis;
	vec4d yAxis;
	vec4d zAxis;
};
