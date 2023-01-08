#pragma once

#include <assert.h> 
#include <float.h>
#include <cstdint>
#include <image/bitmap.h>
#include <image/image.h>
#include <image/color.h>
#include <math/vector.h>
#include <primitives/ray.h>
#include <core/common.h>
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
void TracePixel( const RtView& view, const RtScene& rtScene, Image<Color>& image, const uint32_t px, const uint32_t py );
void TracePatch( const RtView& view, const RtScene& rtScene, Image<Color>* image, const vec2i& p0, const vec2i& p1 );
void TraceScene( const RtView& view, const RtScene& rtScene, Image<Color>& image );
void RasterScene( Image<Color>& image, const RtView& view, const RtScene& rtScene, bool wireFrame = true );