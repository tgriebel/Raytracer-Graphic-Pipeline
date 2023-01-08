#pragma once

#include <image/bitmap.h>
#include <image/image.h>
#include <image/color.h>
#include <math/vector.h>
#include <math/matrix.h>
#include "debug.h"

#define USE_AABB		1
#define USE_RELFECTION	0
#define USE_SHADOWS		0
#define USE_RAYTRACE	1
#define USE_SSRAND		0 // TODO: Halton sequence
#define USE_SS4X		0
#define USE_RASTERIZE	1
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
static const uint32_t	RenderWidth			= 360;
static const uint32_t	RenderHeight		= 240;
#elif 1
static const uint32_t	RenderWidth			= 180;
static const uint32_t	RenderHeight		= 120;
#endif
static const float		HalfRenderWidth		= ( RenderWidth * 0.5f );
static const float		HalfRenderHeight	= ( RenderHeight * 0.5f );
static const float		CameraFov			= 90.0f;
static const float		CameraNearPlane		= 0.1f;
static const float		CameraFarPlane		= 1000.0f;
static const vec2i		RenderSize			= vec2i( RenderWidth, RenderHeight );

static const float		AmbientLight		= 0.1f;
static const float		SpecularPower		= 15.0f;
static const float		MaxT				= 1000.0f;
static const uint32_t	MaxBounces			= 3;

enum axisMode_t : uint32_t
{
	// Hand_Horizontal-Depth-Up
	RHS_XZY,
	RHS_XYZ,
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


inline float AspectRatio( const vec2i& targetSize )
{
	return ( targetSize[ 0 ] / static_cast<float>( targetSize[ 1 ] ) );
}


inline void OrthoMatrixToAxis( const mat4x4f& m, vec3f& origin, vec3f& xAxis, vec3f& yAxis, vec3f& zAxis )
{
	origin = vec3f( m[ 0 ][ 3 ], m[ 1 ][ 3 ], m[ 2 ][ 3 ] );
	xAxis = vec3f( m[ 0 ][ 0 ], m[ 1 ][ 0 ], m[ 2 ][ 0 ] );
	yAxis = vec3f( m[ 0 ][ 1 ], m[ 1 ][ 1 ], m[ 2 ][ 1 ] );
	zAxis = vec3f( m[ 0 ][ 2 ], m[ 1 ][ 2 ], m[ 2 ][ 2 ] );
}


static const Color DbgColors[ 16 ] =
{
	Color( 1.0f, 0.0f, 0.0f ),
	Color( 0.0f, 1.0f, 0.0f ),
	Color( 0.0f, 0.0f, 1.0f ),
	Color( 1.0f, 0.0f, 1.0f ),
	Color( 1.0f, 1.0f, 0.0f ),
	Color( 0.0f, 1.0f, 1.0f ),
	Color( 1.0f, 1.0f, 1.0f ),
	Color( 0.0f, 0.0f, 0.0f ),
	Color( 0.5f, 0.0f, 0.0f ),
	Color( 0.0f, 0.5f, 0.0f ),
	Color( 0.0f, 0.0f, 0.5f ),
	Color( 1.0f, 0.0f, 0.5f ),
	Color( 1.0f, 0.5f, 0.0f ),
	Color( 0.0f, 1.0f, 0.5f ),
	Color( 0.5f, 0.5f, 0.5f ),
	Color( 0.25f, 0.5f, 0.75f ),
};