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

#include <assert.h> 
#include <float.h>
#include <cstdint>
#include <gfxcore/image/bitmap.h>
#include <gfxcore/image/image.h>
#include <gfxcore/image/color.h>
#include <gfxcore/math/vector.h>
#include <gfxcore/primitives/ray.h>
#include <gfxcore/core/common.h>
#include "scene.h"

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
	vec3f		pt;
	vec3f		normal;
	float		t;
	float		surfaceDot;
	uint32_t	modelIx;
	hitCode_t	hitCode;
	hdl_t		materialId;
};


sample_t RayTrace_r( const Ray& ray, const RtScene& rtScene, const uint32_t rayDepth );
sample_t RecordSkyInfo( const Ray& r, const float t );
sample_t RecordSurfaceInfo( const Ray& r, const float t, const RtScene& rtScene, const uint32_t triIndex, const uint32_t modelIx );
bool IntersectScene( const Ray& ray, const RtScene& rtScene, const bool cullBackfaces, const bool stopAtFirstIntersection, sample_t& outSample );
void TracePixel( const RtView& view, const RtScene& rtScene, ImageBuffer<Color>& image, const uint32_t px, const uint32_t py );
void TracePatch( const RtView& view, const RtScene& rtScene, ImageBuffer<Color>* image, const vec2i& p0, const vec2i& p1 );
void TraceScene( const RtView& view, const RtScene& rtScene, ImageBuffer<Color>& image );
void RasterScene( ImageBuffer<Color>& image, const RtView& view, const RtScene& rtScene, bool wireFrame = true );