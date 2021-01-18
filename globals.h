#pragma once

#include "../GfxCore/bitmap.h"
#include "../GfxCore/mathVector.h"
#include "../GfxCore/matrix.h"
#include "../GfxCore/color.h"
#include "../GfxCore/image.h"
#include "debug.h"

#define USE_AABB		1
#define USE_RELFECTION	1
#define USE_SHADOWS		1
#define USE_RAYTRACE	1
#define USE_SS4X		1
#define USE_RASTERIZE	0
#define DRAW_WIREFRAME	1
#define	DRAW_AABB		1
#define PHONG_NORMALS	1
// TODO: winding order support

#if 0
static const uint32_t	RenderWidth			= 1920;
static const uint32_t	RenderHeight		= 1080;
#elif 1
static const uint32_t	RenderWidth			= 720;
static const uint32_t	RenderHeight		= 480;
#elif 1
static const uint32_t	RenderWidth			= 180;
static const uint32_t	RenderHeight		= 120;
#endif
static const double		HalfRenderWidth		= ( RenderWidth * 0.5 );
static const double		HalfRenderHeight	= ( RenderHeight * 0.5 );
static const double		CameraFov			= 90.0f;
static const double		CameraNearPlane		= 0.1f;
static const double		CameraFarPlane		= 1000.0f;
static const vec2i		RenderSize			= vec2i( RenderWidth, RenderHeight );

static const float		AmbientLight		= 0.1f;
static const double		SpecularPower		= 15.0;
static const double		MaxT				= 1000.0;
static const uint32_t	MaxBounces			= 3;

enum axisMode_t : uint32_t
{
	// Hand_Horizontal-Depth-Up
	RHS_XZY,
	RHS_XYZ,
};


enum hitCode_t : uint32_t
{
	HIT_NONE,
	HIT_SKY,
	HIT_AABB,
	HIT_FRONTFACE,
	HIT_BACKFACE,
};


struct sample_t
{
	Color		color;
	Color		albedo;
	vec3d		pt;
	vec3d		normal;
	double		t;
	double		surfaceDot;
	uint32_t	modelIx;
	hitCode_t	hitCode;
	uint32_t	materialId;
};


struct debug_t
{
	Image<Color> diffuse;
	Image<Color> normal;
	Image<Color> wireframe;
	Image<Color> topWire;
	Image<Color> sideWire;
};


enum viewType_t
{
	VIEW_CAMERA = 0,
	VIEW_FRONT	= 1,
	VIEW_TOP	= 2,
	VIEW_SIDE	= 3,
};


inline double AspectRatio( const vec2i& targetSize )
{
	return ( targetSize[ 0 ] / static_cast<double>( targetSize[ 1 ] ) );
}


inline Color Vec3dToColor( const vec3d& v )
{
	return Color( static_cast<float>( v[ 0 ] ), static_cast<float>( v[ 1 ] ), static_cast<float>( v[ 2 ] ), 1.0f );
}


inline Color Vec4dToColor( const vec4d& v )
{
	return Color( static_cast<float>( v[ 0 ] ), static_cast<float>( v[ 1 ] ), static_cast<float>( v[ 2 ] ), static_cast<float>( v[ 3 ] ) );
}


inline vec4d ColorToVector( const Color& c )
{
	return vec4d( c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ] );
}


inline void OrthoMatrixToAxis( const mat4x4d& m, vec3d& origin, vec3d& xAxis, vec3d& yAxis, vec3d& zAxis )
{
	origin = vec3d( m[ 0 ][ 3 ], m[ 1 ][ 3 ], m[ 2 ][ 3 ] );
	xAxis = vec3d( m[ 0 ][ 0 ], m[ 1 ][ 0 ], m[ 2 ][ 0 ] );
	yAxis = vec3d( m[ 0 ][ 1 ], m[ 1 ][ 1 ], m[ 2 ][ 1 ] );
	zAxis = vec3d( m[ 0 ][ 2 ], m[ 1 ][ 2 ], m[ 2 ][ 2 ] );
}