/*
* MIT License
*
* Copyright( c ) 2023 Thomas Griebel
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this softwareand associated documentation files( the "Software" ), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <gfxcore/image/bitmap.h>
#include <gfxcore/image/image.h>
#include <gfxcore/image/color.h>
#include <gfxcore/math/vector.h>
#include <gfxcore/math/matrix.h>
#include "debug.h"

#define USE_AABB		1
#define USE_RELFECTION	0
#define USE_SHADOWS		0
#define USE_RAYTRACE	1
#define USE_RAYCAST		0
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
	CpuImage<Color> diffuse;
	CpuImage<Color> normal;
	CpuImage<Color> wireframe;
	CpuImage<Color> topWire;
	CpuImage<Color> sideWire;
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