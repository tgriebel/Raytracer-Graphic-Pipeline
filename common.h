#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "mathVector.h"
#include "matrix.h"
#include "color.h"

// TODO: move
struct material_t
{
	double Ka;
	double Kt;
	double Kd;
	double Ks;
	double Kr;
};


template<typename T>
inline T Clamp( const T& number, const T& min, const T& max )
{
	const T tmp = ( number < min ) ? min : number;
	return ( tmp > max ) ? max : tmp;
}


template<typename T>
inline T Saturate( const T& number )
{
	return Clamp( number, static_cast<T>( 0.0 ), static_cast<T>( 1.0 ) );
}


template<typename T1, typename T2>
inline T1 Lerp( const T1& v0, const T1& v1, T2 t )
{
	t = Saturate( t );
	T1 one = T1( static_cast<T2>( 1.0 ) );
	return ( one - t ) * v0 + t * v1;
}


template<typename T>
inline T Radians( const T& degrees )
{
	return ( degrees * ( PI / 180.0 ) );
}


template<typename T>
inline T Degrees( const T& radians )
{
	return ( radians * ( 180.0 / PI ) );
}


inline mat4x4d ComputeRotationX( const double degrees )
{
	const double theta = Radians( degrees );
	return CreateMatrix4x4( 1.0,	0.0,			0.0,			0.0,
							0.0,	cos( theta ),	-sin( theta ),	0.0,
							0.0,	sin( theta ),	cos( theta ),	0.0,
							0.0,	0.0,			0.0,			1.0 );
}


inline mat4x4d ComputeRotationY( const double degrees )
{
	const double theta = Radians( degrees );
	return CreateMatrix4x4(	cos( theta ),	0.0,	sin( theta ),	0.0,
							0.0,			1.0,	0.0,			0.0,
							-sin( theta ),	0.0,	cos( theta ),	0.0,
							0.0,			0.0,	0.0,			1.0 );
}


inline mat4x4d ComputeRotationZ( const double degrees )
{
	const double theta = Radians( degrees );
	return CreateMatrix4x4( cos( theta ),	-sin( theta ),	0.0,	0.0,
							sin( theta ),	cos( theta ),	0.0,	0.0,
							0.0,			0.0,			1.0,	0.0,
							0.0,			0.0,			0.0,	1.0 );
}


inline void SetTranslation( mat4x4d& inoutMatrix, const vec3d& translation )
{
	inoutMatrix[ 0 ][ 3 ] = translation[ 0 ];
	inoutMatrix[ 1 ][ 3 ] = translation[ 1 ];
	inoutMatrix[ 2 ][ 3 ] = translation[ 2 ];
	inoutMatrix[ 3 ][ 3 ] = 1.0;
}


inline mat4x4d ComputeScale( const vec3d& scale )
{
	mat4x4d mat;
	mat[ 0 ][ 0 ] = scale[ 0 ];
	mat[ 1 ][ 1 ] = scale[ 1 ];
	mat[ 2 ][ 2 ] = scale[ 2 ];
	mat[ 3 ][ 3 ] = 1.0;
	return mat;
}