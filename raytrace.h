#pragma once

#include <assert.h> 
#include <float.h>
#include <cstdint>
#include "../GfxCore/src/primitives/ray.h"
#include "../GfxCore/src/core/common.h"
#include "../GfxCore/src/image/color.h"
#include "../GfxCore/src/image/image.h"
#include "../GfxCore/src/math/mathvector.h"

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
	int32_t		materialId;
};


sample_t RayTrace_r( const Ray& ray, const uint32_t rayDepth );
sample_t RecordSkyInfo( const Ray& r, const float t );
sample_t RecordSurfaceInfo( const Ray& r, const float t, const uint32_t triIndex, const uint32_t modelIx );
bool IntersectScene( const Ray& ray, const bool cullBackfaces, const bool stopAtFirstIntersection, sample_t& outSample );
void TracePixel( const SceneView& view, Image<Color>& image, const uint32_t px, const uint32_t py );
void TracePatch( const SceneView& view, Image<Color>* image, const vec2i& p0, const vec2i& p1 );