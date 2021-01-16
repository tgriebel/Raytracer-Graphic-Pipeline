#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "../GfxCore/mathVector.h"
#include "../GfxCore/matrix.h"
#include "../GfxCore/color.h"
#include "../GfxCore/common.h"

// TODO: move
struct material_t
{
	double		Ka;
	double		Ke;
	double		Kd;
	double		Ks;
	double		Tf;
	double		Tr;
	double		Ns;
	double		Ni;
	double		d;
	double		illum;

	bool		textured;
	int32_t		colorMapId;
	int32_t		normalMapId;
};


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