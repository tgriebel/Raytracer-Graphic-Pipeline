#include <iostream>
#include <fstream>
#include <cmath>
#include <queue>
#include <map>
#include <sstream>
#include <assert.h> 
#include <float.h>
#include <cstdint>
#include <tuple>
#include <map>
#include <thread>
#include <image/bitmap.h>
#include <image/image.h>
#include <image/color.h>
#include <math/vector.h>
#include <math/matrix.h>
#include <io/meshIO.h>
#include <primitives/geom.h>
#include <scene/resourceManager.h>
#include <scene/camera.h>
#include <core/rasterLib.h>
#include <core/util.h>
#include "scene.h"
#include "debug.h"
#include "globals.h"
#include "timer.h"
#include "raytrace.h"


extern debug_t			dbg;
extern ResourceManager	rm;
extern RtScene			scene;
extern SceneView		views[ 4 ];
extern debug_t			dbg;
extern Image<Color>		colorBuffer;
extern Image<float>		depthBuffer;

static Color skyColor = Color::Blue;

sample_t RecordSkyInfo( const Ray& r, const float t )
{
	sample_t sample;

	const float skyDot = Saturate( Dot( r.GetVector(), vec3f( 0.0, 0.0, 1.0 ) ) );
	const Color gradient = Lerp( Color( Color::White ), skyColor, Saturate( skyDot ) );

	sample.color = gradient;
	sample.albedo = gradient;
	sample.normal = vec3f( 0.0 );
	sample.hitCode = HIT_SKY;
	sample.modelIx = ResourceManager::InvalidModelIx;
	sample.pt = vec3f( 0.0 );
	sample.surfaceDot = 0.0;
	sample.t = t;
	sample.materialId = 0;

	return sample;
}


sample_t RecordSurfaceInfo( const Ray& r, const float t, const uint32_t triIndex, const uint32_t modelIx )
{
	const RtModel& model = scene.models[ modelIx ];
	const std::vector<Triangle>& triCache = model.triCache;
	const Triangle& tri = triCache[ triIndex ];

	sample_t sample;

	sample.pt = r.GetPoint( t );
	sample.t = t;

	const vec3f b = PointToBarycentric( sample.pt, Trunc<4, 1>( tri.v0.pos ), Trunc<4, 1>( tri.v1.pos ), Trunc<4, 1>( tri.v2.pos ) );
#if PHONG_NORMALS
	sample.normal = ( b[ 0 ] * tri.v0.normal ) + ( b[ 1 ] * tri.v1.normal ) + ( b[ 2 ] * tri.v2.normal );
	sample.normal = sample.normal.Normalize();
#else
	sample.normal = tri.n;
#endif

	vec4f color0 = ColorToVector( tri.v0.color );
	vec4f color1 = ColorToVector( tri.v1.color );
	vec4f color2 = ColorToVector( tri.v2.color );
	vec4f mixedColor = b[ 0 ] * color0 + b[ 1 ] * color1 + b[ 2 ] * color2;
	sample.color = Vec4ToColor( mixedColor );

	sample.albedo = sample.color;

	sample.materialId = tri.materialId;

	const material_t* material = rm.GetMaterialRef( sample.materialId );
	if ( ( material != nullptr ) && material->textured )
	{
		const Image<Color>* texture = rm.GetImageRef( material->colorMapId );
		vec2f uv = b[ 0 ] * tri.v0.uv + b[ 1 ] * tri.v1.uv + b[ 2 ] * tri.v2.uv;
		sample.albedo = texture->GetPixel( static_cast<int32_t>( uv[ 0 ] * texture->GetWidth() ),
			static_cast<int32_t>( uv[ 1 ] * texture->GetHeight() ) );
	}

	sample.surfaceDot = Dot( r.GetVector(), sample.normal );
	if ( sample.surfaceDot > 0.0 )
	{
		sample.hitCode = HIT_BACKFACE;
	}
	else
	{
		sample.hitCode = HIT_FRONTFACE;
	}

	sample.modelIx = modelIx;

	return sample;
}


bool IntersectScene( const Ray& ray, const bool cullBackfaces, const bool stopAtFirstIntersection, sample_t& outSample )
{
	outSample.t = FLT_MAX;
	outSample.hitCode = HIT_NONE;

	int hitCnt = 0;

	const uint32_t modelCnt = static_cast<uint32_t>( scene.models.size() );
	for ( uint32_t modelIx = 0; modelIx < modelCnt; ++modelIx )
	{
		const RtModel& model = scene.models[ modelIx ];

#if USE_AABB
		float t0 = 0.0;
		float t1 = 0.0;
		if ( !model.octree.GetAABB().Intersect( ray, t0, t1 ) )
		{
			continue;
		}
#endif

		const std::vector<Triangle>& triCache = model.triCache;
		std::vector<uint32_t> triIndices;
		model.octree.Intersect( ray, triIndices );

		//const size_t triCnt = triCache.size();
		const size_t triCnt = triIndices.size();
		for ( size_t ix = 0; ix < triCnt; ++ix )
		{
			const uint32_t triIx = triIndices[ ix ];
			// const uint32_t triIx = ix;
			const Triangle& tri = triCache[ triIx ];

			float t;
			bool isBackface;
			if ( RayToTriangleIntersection( ray, tri, isBackface, t ) )
			{
				if ( t > outSample.t )
					continue;

				if ( cullBackfaces && isBackface )
					continue;

				outSample = RecordSurfaceInfo( ray, t, triIx, modelIx );

				if ( stopAtFirstIntersection )
					return true;
			}
		}
	}

	return outSample.hitCode != HIT_NONE;
}


sample_t RayTrace_r( const Ray& ray, const uint32_t rayDepth )
{
	float tnear = 0;
	float tfar = 0;

	sample_t sample;
	sample.color = Color::Black;
	sample.hitCode = HIT_NONE;

#if USE_AABB
	if ( !scene.aabb.Intersect( ray, tnear, tfar ) )
	{
		return sample;
	}
#endif

	sample_t surfaceSample;
	if ( !IntersectScene( ray, true, false, surfaceSample ) )
	{
		sample = RecordSkyInfo( ray, surfaceSample.t );
		return sample;
	}
	else
	{
		Color finalColor = Color::Black;
		const material_t& material = *rm.GetMaterialRef( surfaceSample.materialId );
		Color surfaceColor = material.textured ? surfaceSample.albedo : surfaceSample.color;

		vec3f viewVector = ray.GetVector().Reverse();
		viewVector = viewVector.Normalize();

		Color relfectionColor = Color::Black;
#if USE_RELFECTION
		if ( ( rayDepth < MaxBounces ) && ( material.Tr > 0.0 ) )
		{
			vec3f reflectVector = ReflectVector( surfaceSample.normal, viewVector );
			reflectVector += RandomVector( 0.1f );
			reflectVector = MaxT * reflectVector;

			Ray reflectionRay = Ray( surfaceSample.pt, surfaceSample.pt + reflectVector );

			const sample_t reflectSample = RayTrace_r( reflectionRay, rayDepth + 1 );
			relfectionColor = material.Tr * reflectSample.color;

			sample = surfaceSample;
			sample.color = relfectionColor;

			return sample;
		}
#endif

		const size_t lightCnt = scene.lights.size();
		for ( size_t li = 0; li < lightCnt; ++li )
		{
			light_t& L = scene.lights[ li ];
			vec3f& lightPos = L.pos;

			Ray shadowRay = Ray( surfaceSample.pt, lightPos );

			sample_t shadowSample;
#if USE_SHADOWS
			const bool lightOccluded = IntersectScene( shadowRay, true, true, shadowSample );
#else
			const bool lightOccluded = false;
#endif

			Color shadingColor = Color::Black;
			if ( !lightOccluded )
			{
				const vec4f intensity = vec4f( L.intensity, 1.0f );

				vec3f lightDir = shadowRay.GetVector();
				lightDir = lightDir.Normalize();

				const vec3f halfVector = ( viewVector + lightDir ).Normalize();

				const vec4f D = ColorToVector( Color( material.Kd ) );
				const vec4f S = ColorToVector( Color( material.Ks ) );

				const vec4f diffuseIntensity = Multiply( D, intensity ) * std::max( 0.0f, Dot( lightDir, surfaceSample.normal ) );

				const vec4f specularIntensity = Multiply( S, intensity ) * pow( std::max( 0.0f, Dot( surfaceSample.normal, halfVector ) ), material.Ns );

				shadingColor += Vec4ToColor( specularIntensity );
				shadingColor += Vec4ToColor( Multiply( diffuseIntensity, ColorToVector( surfaceColor ) ) );
			}

			finalColor += shadingColor + relfectionColor;
		}

		const Color ambient = AmbientLight * ( Color( material.Ka ) * surfaceColor );

		sample = surfaceSample;
		sample.color = finalColor + ambient;
		return sample;
	}

	return sample;
}


void TracePixel( const SceneView& view, Image<Color>& image, const uint32_t px, const uint32_t py )
{
#if	USE_SSRAND
	const uint32_t subSampleCnt = 100;
	vec2f subPixelOffsets[ subSampleCnt ];
	for ( uint32_t ri = 0; ri < subSampleCnt; ++ri )
	{
		subPixelOffsets[ ri ] = vec2f( Random(), Random() );
	}
#elif USE_SS4X
	static const uint32_t subSampleCnt = 4;
	static const vec2f subPixelOffsets[ subSampleCnt ] = { vec2f( 0.25, 0.25 ), vec2f( 0.75, 0.25 ), vec2f( 0.25, 0.75 ), vec2f( 0.75, 0.75 ) };
#else
	static const uint32_t subSampleCnt = 1;
	static const vec2f subPixelOffsets[ subSampleCnt ] = { vec2f( 0.5, 0.5 ) };
#endif

	Color pixelColor = Color::Black;
	vec3f normal = vec3f( 0.0, 0.0, 0.0 );
	float diffuse = 0.0; // Eye-to-Surface
	float coverage = 0.0;
	float t = 0.0;

	sample_t sample;
	sample.hitCode = HIT_NONE;

	for ( int32_t s = 0; s < subSampleCnt; ++s ) // Subsamples
	{
		vec2f pixelXY = vec2f( static_cast<float>( px ), static_cast<float>( py ) );
		pixelXY += subPixelOffsets[ s ];
		vec2f uv = vec2f( pixelXY[ 0 ] / ( view.targetSize[ 0 ] - 1.0f ), pixelXY[ 1 ] / ( view.targetSize[ 1 ] - 1.0f ) );

		Ray ray = view.camera.GetViewRay( uv );

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// Experimental
		const vec3f up = vec3f( 0.0, 0.0, 1.0 );
		const vec3f z = ray.GetVector();
		const vec3f x = Cross( up, z );
		const vec3f y = Cross( x, z );

		float e0, e1;
		RandomPointOnCircle( e0, e1 );
		const vec4f r = vec4f( e0, e1, 0.0, 0.0 );

		const mat4x4f m = CreateMatrix4x4( x[ 0 ], x[ 1 ], x[ 2 ], 0.0f,
			y[ 0 ], y[ 1 ], y[ 2 ], 0.0f,
			z[ 0 ], z[ 1 ], z[ 2 ], 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f );

		const vec4f perturb = m * r;

		//ray.d = ray.d + Trunc<4,1>( 0.01 * perturb );
		//assert( ( Dot( x, z ) < 1e6 ) && ( Dot( x, y ) < 1e6 ) && ( Dot( y, z ) < 1e6 ) );
		//////////////////////////////////////////////////////////////////////////////////////////////////////

		sample = RayTrace_r( ray, 0 );
		pixelColor += sample.color;
		diffuse += sample.surfaceDot;
		normal += sample.normal;
		t += sample.t;
		coverage += sample.hitCode != HIT_NONE ? 1.0f : 0.0f;
	}

	if ( coverage > 0.0 )
	{
		int32_t imageX = static_cast<int32_t>( px );
		int32_t imageY = static_cast<int32_t>( py );

		coverage /= subSampleCnt;
		diffuse /= subSampleCnt;
		normal = normal.Normalize();
		t /= subSampleCnt;

		Color src = Color( LinearToSrgb( ( 1.0f / subSampleCnt ) * pixelColor ) );
		src.rgba().a = (float)coverage;

		// normal = normal.Reverse();
		Color normColor = Vec4ToColor( vec4f( 0.5f * normal + vec3f( 0.5f ), 1.0f ) );
		dbg.diffuse.SetPixel( imageX, imageY, Color( (float)-diffuse ).AsR8G8B8A8() );
		dbg.normal.SetPixel( imageX, imageY, normColor.AsR8G8B8A8() );

		Color dest = Color( image.GetPixel( imageX, imageY ) );

		Color pixel = BlendColor( src, dest, blendMode_t::SRCALPHA );
		image.SetPixel( imageX, imageY, pixel );
	}
}


void TracePatch( const SceneView& view, Image<Color>* image, const vec2i& p0, const vec2i& p1 )
{
	const int32_t x0 = p0[ 0 ];
	const int32_t y0 = p0[ 1 ];
	const int32_t x1 = p1[ 0 ];
	const int32_t y1 = p1[ 1 ];

	if ( ( y1 < 0 ) || ( x1 < 0 ) ) {
		return;
	}

	for ( uint32_t py = y0; py < static_cast<uint32_t>( y1 ); ++py )
	{
		if ( py >= image->GetHeight() )
			return;

		for ( uint32_t px = x0; px < static_cast<uint32_t>( x1 ); ++px )
		{
			if ( px >= image->GetWidth() )
				return;

			TracePixel( view, *image, px, py );
		}
	}
}