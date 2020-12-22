
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
#include "common.h"
#include "bitmap.h"
#include "color.h"
#include "mathVector.h"
#include "matrix.h"
#include "meshIO.h"
#include "rasterLib.h"
#include "geom.h"
#include "model.h"
#include "resourceManager.h"
#include "camera.h"
#include "scene.h"
#include "debug.h"

#if 0
const uint32_t	RenderWidth			= 1920;
const uint32_t	RenderHeight		= 1080;
#elif 0
const uint32_t	RenderWidth = 720;
const uint32_t	RenderHeight = 480;
#elif 1
const uint32_t	RenderWidth = 180;
const uint32_t	RenderHeight = 120;
#endif
const double	HalfRenderWidth		= ( RenderWidth * 0.5 );
const double	HalfRenderHeight	= ( RenderHeight * 0.5 );
const double	AspectRatio			= RenderWidth / static_cast<double>( RenderHeight );
const double	CameraFov			= 90.0f;
const double	CameraNearPlane		= 0.0001f;
const double	CameraFarPlane		= 1000.0f;
const vec2i		RenderSize			= vec2i( RenderWidth, RenderHeight );

#define USE_AABB		1
#define USE_RELFECTION	0
#define USE_SHADOWS		1
#define USE_RAYTRACE	1
#define USE_SS4X		0
#define DRAW_WIREFRAME	1
#define	DRAW_AABB		1
#define PHONG_NORMALS	1
// TODO: winding order support

const float		AmbientLight	= 0.2f;
const double	SpecularPower	= 15.0;
const double	MaxT			= 10000.0;
const uint32_t	MaxBounces		= 1;

enum axisMode_t : uint32_t
{
	// Hand_Horizontal-Depth-Up
	RHS_XZY,
	RHS_XYZ,
};


enum hitCode_t : uint32_t
{
	HIT_NONE,
	HIT_AABB,
	HIT_FRONTFACE,
	HIT_BACKFACE,
};

ResourceManager	rm;


struct sample_t
{
	Color		color;
	vec3d		pt;
	vec3d		normal;
	double		t;
	double		surfaceDot;
	uint32_t	modelIx;
	hitCode_t	hitCode;
};


struct debug_t
{
	Bitmap* diffuse;
	Bitmap* normal;
	Bitmap* wireframe;
	Bitmap* topWire;
	Bitmap* sideWire;
};


enum viewType_t
{
	VIEW_CAMERA = 0,
	VIEW_TOP	= 1,
	VIEW_SIDE	= 2,
};


material_t mirrorMaterial = { 1.0, 0.5, 0.5, 1.0, 1.0 };

Scene			scene;
SceneView		views[4];
debug_t			dbg;


Color VectorToColor( const vec4d& v )
{
	return Color( static_cast<float>( v[ 0 ] ), static_cast<float>( v[ 1 ] ), static_cast<float>( v[ 2 ] ), static_cast<float>( v[ 3 ] ) );
}


vec4d ColorToVector( const Color& c )
{
	return vec4d( c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ] );
}


void OrthoMatrixToAxis( const mat4x4d& m, vec3d& origin, vec3d& xAxis, vec3d& yAxis, vec3d& zAxis )
{
	origin = vec3d( m[ 0 ][ 3 ], m[ 1 ][ 3 ], m[ 2 ][ 3 ] );
	xAxis = vec3d( m[ 0 ][ 0 ], m[ 1 ][ 0 ], m[ 2 ][ 0 ] );
	yAxis = vec3d( m[ 0 ][ 1 ], m[ 1 ][ 1 ], m[ 2 ][ 1 ] );
	zAxis = vec3d( m[ 0 ][ 2 ], m[ 1 ][ 2 ], m[ 2 ][ 2 ] );
}


void CreatePlaneCodeSurf( std::vector<vertex_t>& vb, std::vector<uint32_t>& indices, const vec3d origin, const uint32_t widthInQuads, const uint32_t heightInQuads, const double gridSize )
{
	std::pair<size_t, size_t> vertexDim = std::pair<size_t, size_t>( widthInQuads + 1, heightInQuads + 1 );

	const size_t verticesPerQuad = 6;

	const size_t firstIndex = vb.size();
	size_t indicesCnt = indices.size();
	size_t vbIx = firstIndex;
	vb.resize( vbIx + vertexDim.first * vertexDim.second );
	indices.resize( indicesCnt + verticesPerQuad * widthInQuads * heightInQuads );

	for ( size_t j = 0; j < vertexDim.second; ++j )
	{
		for ( size_t i = 0; i < vertexDim.first; ++i )
		{
			vertex_t& vert = vb[ vbIx ];

			vert.pos = { i * gridSize, j * gridSize, 0.0f };
			vert.color = { 0.0f, 1.0f, 1.0f };
			vert.normal = { 0.0f, 0.0f, 1.0f };

			vert.pos -= Concat<3,1>( origin );

			++vbIx;
		}
	}

	for ( uint32_t j = 0; j < heightInQuads; ++j )
	{
		for ( uint32_t i = 0; i < widthInQuads; ++i )
		{
			uint32_t vIx[ 4 ];
			vIx[ 0 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 0 ) * ( heightInQuads + 1 ) );
			vIx[ 1 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 0 ) * ( heightInQuads + 1 ) );
			vIx[ 2 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 1 ) * ( heightInQuads + 1 ) );
			vIx[ 3 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 1 ) * ( heightInQuads + 1 ) );

			indices[ indicesCnt++ ] = vIx[ 2 ];
			indices[ indicesCnt++ ] = vIx[ 1 ];
			indices[ indicesCnt++ ] = vIx[ 0 ];

			indices[ indicesCnt++ ] = vIx[ 2 ];
			indices[ indicesCnt++ ] = vIx[ 3 ];
			indices[ indicesCnt++ ] = vIx[ 1 ];
		}
	}
}

void DrawCube( Bitmap& bitmap, const SceneView& view, const vec4d& minCorner, const vec4d& maxCorner )
{
	vec4d corners[ 8 ] = {
		// Bottom
		vec4d( minCorner[ 0 ], minCorner[ 1 ], minCorner[ 2 ], 1.0 ),
		vec4d( maxCorner[ 0 ], minCorner[ 1 ], minCorner[ 2 ], 1.0 ),
		vec4d( maxCorner[ 0 ], maxCorner[ 1 ], minCorner[ 2 ], 1.0 ),
		vec4d( minCorner[ 0 ], maxCorner[ 1 ], minCorner[ 2 ], 1.0 ),
		// Top
		vec4d( minCorner[ 0 ], minCorner[ 1 ], maxCorner[ 2 ], 1.0 ),
		vec4d( maxCorner[ 0 ], minCorner[ 1 ], maxCorner[ 2 ], 1.0 ),
		vec4d( maxCorner[ 0 ], maxCorner[ 1 ], maxCorner[ 2 ], 1.0 ),
		vec4d( minCorner[ 0 ], maxCorner[ 1 ], maxCorner[ 2 ], 1.0 ),
	};

	int edges[ 12 ][ 2 ] = {
		// Bottom
		{ 0, 1 },
		{ 0, 3 },
		{ 1, 2 },
		{ 2, 3 },
		// Top
		{ 4, 5 },
		{ 4, 7 },
		{ 5, 6 },
		{ 4, 7 },
		// Sides
		{ 0, 4 },
		{ 1, 5 },
		{ 2, 6 },
		{ 3, 7 },
	};

	vec2i ssPts[ 8 ];

	for ( int i = 0; i < 8; ++i )
	{
		vec2d pt;
		ProjectPoint( view.projView, RenderSize, true, corners[ i ], pt );
		ssPts[ i ] = vec2i( static_cast<int32_t>( pt[ 0 ] ), static_cast<int32_t>( pt[ 1 ] ) );
	}

	for ( int i = 0; i < 12; ++i )
	{
		vec2i  pt0 = ssPts[ edges[ i ][ 0 ] ];
		vec2i  pt1 = ssPts[ edges[ i ][ 1 ] ];
		DrawLine( bitmap, pt0[ 0 ], pt0[ 1 ], pt1[ 0 ], pt1[ 1 ], Color::Green );
	}

	for ( int i = 0; i < 8; ++i )
	{
		bitmap.SetPixel( ssPts[ i ][ 0 ], ssPts[ i ][ 1 ], Color::Red );
	}
}


void DrawWorldAxis( Bitmap& bitmap, const SceneView& view, double size, const vec3d& origin, const vec3d& X, const vec3d& Y, const vec3d& Z )
{
	vec4d points[ 4 ] = {
		{ vec4d( origin, 1.0 ) },
		{ vec4d( origin + size * X.Normalize(), 1.0 ) },
		{ vec4d( origin + size * Y.Normalize(), 1.0 ) },
		{ vec4d( origin + size * Z.Normalize(), 1.0 ) },
	};

	vec2i ssPts[ 4 ];
	for ( int i = 0; i < 4; ++i )
	{
		vec2d pt;
		ProjectPoint( view.projView, RenderSize, true, points[ i ], pt );
		ssPts[ i ] = vec2i( static_cast<int32_t>( pt[ 0 ] ), static_cast<int32_t>( pt[ 1 ] ) );
	}
	
	DrawLine( bitmap, ssPts[ 0 ][ 0 ], ssPts[ 0 ][ 1 ], ssPts[ 1 ][ 0 ], ssPts[ 1 ][ 1 ], Color::Red );
	DrawLine( bitmap, ssPts[ 0 ][ 0 ], ssPts[ 0 ][ 1 ], ssPts[ 2 ][ 0 ], ssPts[ 2 ][ 1 ], Color::Green );
	DrawLine( bitmap, ssPts[ 0 ][ 0 ], ssPts[ 0 ][ 1 ], ssPts[ 3 ][ 0 ], ssPts[ 3 ][ 1 ], Color::Blue );
	bitmap.SetPixel( ssPts[ 0 ][ 0 ], ssPts[ 0 ][ 1 ], Color::Black );
}


void DrawWorldPoint( Bitmap& bitmap, const SceneView& view, const vec4d& point, const int32_t size, const Color& color )
{
	vec2d projPt;
	ProjectPoint( view.projView, vec2i( RenderWidth, RenderHeight ), true, point, projPt );

	vec2i ssPt = vec2i( static_cast<int32_t>( projPt[0] ), static_cast<int32_t>( projPt[ 1 ] ) );
	vec2d halfPt = vec2d( 0.5 * ( (double)size + 0.5 ) );
	vec2d start = projPt - halfPt;

	int32_t i0 = static_cast<int32_t>( start[ 0 ] );
	int32_t iN = i0 + size;

	int32_t j0 = static_cast<int32_t>( start[ 1 ] );
	int32_t jN = j0 + size;

	for ( int32_t j = j0; j < jN; ++j )
	{
		for ( int32_t i = i0; i < iN; ++i )
		{
			bitmap.SetPixel( i, j, color.AsR8G8B8A8() );
		}
	}
}


void DrawRay( Bitmap& bitmap, const SceneView& view, const Ray& ray, const Color& color )
{
	vec2d ssPt[ 2 ];
	vec4d wsPt[ 2 ];
	wsPt[ 0 ] = vec4d( ray.o, 1.0 );
	wsPt[ 1 ] = vec4d( ray.o + ray.d, 1.0 );
	ProjectPoint( view.projView, vec2i( RenderWidth, RenderHeight ), true, wsPt[ 0 ], ssPt[ 0 ] );
	ProjectPoint( view.projView, vec2i( RenderWidth, RenderHeight ), true, wsPt[ 1 ], ssPt[ 1 ] );
	DrawLine( bitmap, (int) ssPt[ 0 ][ 0 ], (int) ssPt[ 0 ][ 1 ], (int) ssPt[ 1 ][ 0 ], (int) ssPt[ 1 ][ 1 ], color.AsR8G8B8A8() );
}


sample_t RecordSurfaceInfo( const Ray& r, const double t, const uint32_t triIndex, const uint32_t modelIx )
{
	const std::vector<Triangle>& triList = scene.models[ modelIx ].triList;
	const Triangle& tri = triList[ triIndex ];

	sample_t sample;

	sample.pt = r.o + t * r.d;
	sample.t = t;

	const vec3d b = PointToBarycentric( sample.pt, Trunc<4, 1>( tri.v0.pos ), Trunc<4, 1>( tri.v1.pos ), Trunc<4, 1>( tri.v2.pos ) );
#if PHONG_NORMALS
	sample.normal = ( b[ 0 ] * tri.v0.normal ) + ( b[ 1 ] * tri.v1.normal ) + ( b[ 2 ] * tri.v2.normal );
	sample.normal = sample.normal.Normalize();
#else
	sample.normal = tri.n;
#endif

	vec4d color0 = ColorToVector( tri.v0.color );
	vec4d color1 = ColorToVector( tri.v1.color );
	vec4d color2 = ColorToVector( tri.v2.color );
	vec4d mixedColor = b[ 0 ] * color0 + b[ 1 ] * color1 + b[ 2 ] * color2;
	sample.color = VectorToColor( mixedColor );

	sample.surfaceDot = Dot( r.d.Normalize(), sample.normal );
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


bool IntersectScene( Ray& ray, const bool cullBackfaces, const bool stopAtFirstIntersection, sample_t& outSample )
{
	outSample.t = DBL_MAX;
	outSample.hitCode = HIT_NONE;

	const uint32_t modelCnt = scene.models.size();
	for ( uint32_t modelIx = 0; modelIx < modelCnt; ++modelIx )
	{
		const ModelInstance& model = scene.models[ modelIx ];

#if USE_AABB
		double t0 = 0.0;
		double t1 = 0.0;
		if ( !model.aabb.Intersect( ray, t0, t1 ) )
		{
			continue;
		}
#endif

		const std::vector<Triangle>& triList = model.triList;

		const size_t triCnt = triList.size();
		for ( size_t triIx = 0; triIx < triCnt; ++triIx )
		{
			double t;
			bool isBackface;
			if( RayToTriangleIntersection( ray, triList[ triIx ], isBackface, t ) )
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


sample_t RayTrace_r( Ray& ray, const uint32_t bounceNum )
{
	double tnear = 0;
	double tfar = 0;
	
	sample_t sample;
	sample.color = Color::Black;
	sample.hitCode = HIT_NONE;

#if USE_AABB
	if ( !scene.aabb.Intersect( ray, tnear, tfar ) )
	{
		sample.color = Color::Black;
		return sample;
	}
#endif
	 
	sample_t surfaceSample;	
	if( !IntersectScene( ray, true, true, surfaceSample ) )
	{
		sample.color = Color::Black;
		return sample;
	}
	else
	{
		Color finalColor = Color::Black;
		const material_t& material = scene.models[ surfaceSample.modelIx ].material;

		vec3d viewVector = ray.d.Reverse();
		viewVector = viewVector.Normalize();

		const size_t lightCnt = scene.lights.size();
		for ( size_t li = 0; li < lightCnt; ++li )
		{
			vec3d& lightPos = scene.lights[ li ].pos;

			Ray shadowRay = Ray( surfaceSample.pt, lightPos );
			shadowRay.t = DBL_MAX;

//			DrawRay( *dbg.topWire, views[ VIEW_TOP ], shadowRay, Color::Black );
//			DrawRay( *dbg.sideWire, views[ VIEW_SIDE ], shadowRay, Color::Black );

			sample_t shadowSample;
#if USE_SHADOWS
			const bool lightOccluded = IntersectScene( shadowRay, true, true, shadowSample );
#else
			const bool lightOccluded = false;
#endif

			Color shadingColor = Color::Black;

			if ( !lightOccluded )
			{
				const double intensity = scene.lights[ li ].intensity;

				vec3d lightDir = shadowRay.d;
				lightDir = lightDir.Normalize();

				vec3d halfVector = ( viewVector + lightDir ).Normalize();

				const double diffuseIntensity = material.Kd * intensity * std::max( 0.0, Dot( lightDir, surfaceSample.normal ) );

				const double specularIntensity = material.Ks * pow( std::max( 0.0, Dot( surfaceSample.normal, halfVector ) ), SpecularPower );

				shadingColor = ( (float) diffuseIntensity * surfaceSample.color ) + Color( (float) specularIntensity );
			}

			finalColor += shadingColor;
		}

		Color relfectionColor = Color::Black;
#if USE_RELFECTION
		if ( bounceNum < MaxBounces )
		{
			Ray reflectionVector;

			reflectionVector.o = surfaceSample.pt;
			reflectionVector.d = 2.0 * Dot( viewVector, surfaceSample.normal ) * surfaceSample.normal - viewVector;

			const sample_t reflectSample = RayTrace_r( reflectionVector, bounceNum + 1 );
			relfectionColor = reflectSample.color;
		}
#endif
		const Color ambient = AmbientLight * ( (float) material.Ka * surfaceSample.color );

		sample = surfaceSample;
		sample.color = finalColor + relfectionColor + ambient;
		return sample;
	}

	return sample;
}



SceneView SetupCameraView()
{
	SceneView view;

	view.camera = Camera(	vec4d( -280.0, -30.0, 50.0, 0.0 ),
							vec4d( 0.0, -1.0, 0.0, 0.0 ),
							vec4d( 0.0, 0.0, 1.0, 0.0 ),
							vec4d( -1.0, 0.0, 0.0, 0.0 ),
							CameraFov,
							AspectRatio,
							CameraNearPlane,
							CameraFarPlane );

	view.viewTransform = view.camera.ToViewMatrix();
	view.projTransform = view.camera.ToPerspectiveProjMatrix();
	view.projView = view.projTransform * view.viewTransform;

	return view;
}


SceneView SetupTopView()
{
	SceneView view;

	view.camera = Camera(	vec4d( 0.0, 0.0, 280.0, 0.0 ),
							vec4d( 0.0, -1.0, 0.0, 0.0 ),
							vec4d( 1.0, 0.0, 0.0, 0.0 ),
							vec4d( 0.0, 0.0, 1.0, 0.0 ),
							CameraFov,
							AspectRatio,
							CameraNearPlane,
							CameraFarPlane );

	view.viewTransform = view.camera.ToViewMatrix();
	view.projTransform = view.camera.ToPerspectiveProjMatrix();
	view.projView = view.projTransform * view.viewTransform;

	return view;
}


SceneView SetupSideView()
{
	SceneView view;

	view.camera = Camera(	vec4d( 0.0, 280.0, 0.0, 0.0 ),
							vec4d( -1.0, 0.0, 0.0, 0.0 ),
							vec4d( 0.0, 0.0, 1.0, 0.0 ),
							vec4d( 0.0, 1.0, 0.0, 0.0 ),
							CameraFov,
							AspectRatio,
							CameraNearPlane,
							CameraFarPlane );

	view.viewTransform = view.camera.ToViewMatrix();
	view.projTransform = view.camera.ToPerspectiveProjMatrix();
	view.projView = view.projTransform * view.viewTransform;

	return view;
}


void RasterScene( Bitmap& bitmap, const SceneView& view )
{
	mat4x4d mvp = view.projTransform * view.viewTransform;
	const uint32_t modelCnt = scene.models.size();

#if DRAW_WIREFRAME
	for ( uint32_t m = 0; m < modelCnt; ++m )
	{
		const ModelInstance& model = scene.models[ m ];
		const Triangle* triList = model.triList.data();

		const size_t triCnt = model.triList.size();
		for ( uint32_t i = 0; i < triCnt; ++i )
		{
			const vec4d* wsPts[ 3 ];
			vec2d ssPts[ 3 ];
			int culled = 0;

			wsPts[ 0 ] = &triList[ i ].v0.pos;
			wsPts[ 1 ] = &triList[ i ].v1.pos;
			wsPts[ 2 ] = &triList[ i ].v2.pos;

			culled += ProjectPoint( mvp, RenderSize, true, *wsPts[ 0 ], ssPts[ 0 ] );
			culled += ProjectPoint( mvp, RenderSize, true, *wsPts[ 1 ], ssPts[ 1 ] );
			culled += ProjectPoint( mvp, RenderSize, true, *wsPts[ 2 ], ssPts[ 2 ] );

			if ( culled >= 3 )
			{
				continue;
			}

			vec2i pxPts[ 3 ];
			pxPts[ 0 ] = vec2i( static_cast<int32_t>( ssPts[ 0 ][ 0 ] ), static_cast<int32_t>( ssPts[ 0 ][ 1 ] ) );
			pxPts[ 1 ] = vec2i( static_cast<int32_t>( ssPts[ 1 ][ 0 ] ), static_cast<int32_t>( ssPts[ 1 ][ 1 ] ) );
			pxPts[ 2 ] = vec2i( static_cast<int32_t>( ssPts[ 2 ][ 0 ] ), static_cast<int32_t>( ssPts[ 2 ][ 1 ] ) );

#if 0
			AABB ssBox;
			for ( int i = 0; i < 3; ++i )
			{
				ssBox.Expand( vec3d( ssPts[ i ], 0.0 ) );
			}

			const vec3d tPt0 = vec3d( ssPts[ 0 ], 0.0 );
			const vec3d tPt1 = vec3d( ssPts[ 1 ], 0.0 );
			const vec3d tPt2 = vec3d( ssPts[ 2 ], 0.0 );

			const int32_t x0 = static_cast<int>( ssBox.min[ 0 ] );
			const int32_t x1 = static_cast<int>( ssBox.max[ 0 ] + 0.5 );
			const int32_t y0 = static_cast<int>( ssBox.min[ 1 ] );
			const int32_t y1 = static_cast<int>( ssBox.max[ 1 ] + 0.5 );

			for ( int32_t y = y0; y <= y1; ++y )
			{
				for ( int32_t x = x0; x <= x1; ++x )
				{
					const vec3d baryPt = PointToBarycentric( vec3d( x, y, 0.0 ), tPt0, tPt1, tPt2 );

					if ( ( baryPt[ 0 ] >= 0 ) && ( baryPt[ 1 ] >= 0 ) && ( baryPt[ 2 ] >= 0 ) )
					{
						vec3d normal = ( baryPt[ 0 ] * triList[ i ].v0.normal ) + ( baryPt[ 1 ] * triList[ i ].v1.normal ) + ( baryPt[ 2 ] * triList[ i ].v2.normal );
						normal = normal.Normalize();

						const uint32_t color = Color( normal[ 0 ], normal[ 1 ], normal[ 2 ] ).AsHexColor();
						bitmap.SetPixel( x, y, color );
					}
				}
			}
#endif

			DrawLine( bitmap, pxPts[ 0 ][ 0 ], pxPts[ 0 ][ 1 ], pxPts[ 1 ][ 0 ], pxPts[ 1 ][ 1 ], triList[ i ].v0.color );
			DrawLine( bitmap, pxPts[ 0 ][ 0 ], pxPts[ 0 ][ 1 ], pxPts[ 2 ][ 0 ], pxPts[ 2 ][ 1 ], triList[ i ].v0.color );
			DrawLine( bitmap, pxPts[ 1 ][ 0 ], pxPts[ 1 ][ 1 ], pxPts[ 2 ][ 0 ], pxPts[ 2 ][ 1 ], triList[ i ].v0.color );
		}
	}
#endif

	for ( uint32_t m = 0; m < modelCnt; ++m )
	{
		const ModelInstance& model = scene.models[ m ];
#if DRAW_AABB
		DrawCube( bitmap, view, vec4d( model.aabb.min, 1.0 ), vec4d( model.aabb.max, 1.0 ) );
		DrawCube( bitmap, view, vec4d( model.aabb.min, 1.0 ), vec4d( model.aabb.max, 1.0 ) );
#endif
		vec3d origin;
		vec3d xAxis;
		vec3d yAxis;
		vec3d zAxis;
		OrthoMatrixToAxis( model.transform, origin, xAxis, yAxis, zAxis );
		DrawWorldAxis( bitmap, view, 20.0, origin, xAxis, yAxis, zAxis );
	}
}


void DrawScene( Bitmap& bitmap )
{
	views[ VIEW_CAMERA ] = SetupCameraView();
	views[ VIEW_TOP ] = SetupTopView();
	views[ VIEW_SIDE ] = SetupSideView();

#if USE_SS4X
	const uint32_t subSampleCnt = 4;
	const vec2d subPixelOffsets[ subSampleCnt ] = { vec2d( 0.25, 0.25 ), vec2d( 0.75, 0.25 ), vec2d( 0.25, 0.75 ), vec2d( 0.75, 0.75 ) };
#else
	const uint32_t subSampleCnt = 1;
	const vec2d subPixelOffsets[ subSampleCnt ] = { vec2d( 0.5, 0.5 ) };
#endif

#if USE_RAYTRACE
	for ( uint32_t py = 0; py < RenderHeight; ++py )
	{
		for ( uint32_t px = 0; px < RenderWidth; ++px )
		{
			Color pixelColor = Color::Black;
			vec3d normal = vec3d( 0.0, 0.0, 0.0 );
			double diffuse = 0.0; // Eye-to-Surface
			double coverage = 0.0;

			sample_t sample;
			sample.hitCode = HIT_NONE;

			for ( int32_t s = 0; s < subSampleCnt; ++s ) // Subsamples
			{
				vec2d pixelXY = vec2d( static_cast<double>( px ), static_cast<double>( py ) );
				pixelXY += subPixelOffsets[ s ];
				vec2d uv = vec2d( pixelXY[0] / ( RenderWidth - 1.0 ), pixelXY[1] / ( RenderHeight - 1.0 ) );

				Ray ray = views[ VIEW_CAMERA ].camera.GetViewRay( uv );

				sample = RayTrace_r( ray, 0 );
				pixelColor += sample.color;
				diffuse += sample.surfaceDot;
				normal += sample.normal;
				coverage += sample.hitCode != HIT_NONE ? 1.0 : 0.0;
			}
		
			if ( coverage > 0.0 )
			{
				int32_t imageX = static_cast<int32_t>( px );
				int32_t imageY = static_cast<int32_t>( RenderHeight - py - 1 );

				coverage /= subSampleCnt;
				diffuse /= subSampleCnt;
				normal = normal.Normalize();

				Color src = Color( LinearToSrgb( ( 1.0f / subSampleCnt ) * pixelColor ) );
				src.Rgba().a = (float) coverage;

				// normal = normal.Reverse();
				Color normColor = VectorToColor( vec4d( normal, 1.0 ) );
				Pixel dbgPixel = normColor.AsRGBA();
				dbg.diffuse->SetPixel( imageX, imageY, Color( (float)-diffuse ).AsR8G8B8A8() );
				dbg.normal->SetPixel( imageX, imageY, dbgPixel.r8g8b8a8 );

				Color dest = Color( bitmap.GetPixel( imageX, imageY ) );

				Pixel pixel = BlendColor( src, dest, blendMode_t::SRCALPHA ).AsR8G8B8A8();
				bitmap.SetPixel( imageX, imageY, pixel.r8g8b8a8 );
			}
		}
		std::cout << static_cast<int>( 100.0 * ( py / (double) bitmap.GetHeight() ) ) << "% ";
	}
#endif

	RasterScene( *dbg.wireframe, views[ VIEW_CAMERA ] );
	RasterScene( *dbg.topWire, views[ VIEW_TOP ] );
	RasterScene( *dbg.sideWire, views[ VIEW_SIDE ] );
}


uint32_t LoadModel( std::string path, const uint32_t vb, const uint32_t ib )
{
	GeomReader::Off offMesh;
	GeomReader::ReadOFF( path, offMesh );

	uint32_t modelIx = rm.AllocModel();
	Model* model = rm.GetModel( modelIx );

	model->name = path;
	model->vb = vb;
	model->ib = ib;
	model->vbOffset = rm.GetVbOffset( vb );
	model->ibOffset = rm.GetIbOffset( ib );

	for ( int i = 0; i < offMesh.nvertices; ++i )
	{
		vertex_t v;
		v.pos = vec4d( offMesh.vertices[ i ].x, offMesh.vertices[ i ].y, offMesh.vertices[ i ].z, 1.0 );

		Color c( (float)offMesh.vertices[ i ].r, (float) offMesh.vertices[ i ].b, (float) offMesh.vertices[ i ].g, (float)offMesh.vertices[ i ].a );
		v.color = c.AsR8G8B8A8();

		rm.AddVertex( vb, v );
	}
	model->vbEnd = rm.GetVbOffset( vb );

	const size_t triCnt = offMesh.nfaces;
	for ( size_t i = 0; i < triCnt; ++i )
	{
		const GeomReader::Polytope& face = offMesh.faces[ i ];

		rm.AddIndex( ib, model->vbOffset + face.points[ 0 ] );
		rm.AddIndex( ib, model->vbOffset + face.points[ 1 ] );
		rm.AddIndex( ib, model->vbOffset + face.points[ 2 ] );
	}
	model->ibEnd = rm.GetIbOffset( ib );

	model->material.Ka = 1.0;
	model->material.Kt = 0.5;
	model->material.Kd = 0.5;
	model->material.Ks = 1.0;
	model->material.Kr = 1.0;

	return modelIx;
}


uint32_t CreatePlaneModel( const uint32_t vb, const uint32_t ib, const vec2d& size, const vec2i& cellCnt, const material_t& material )
{
	uint32_t modelIx = rm.AllocModel();
	Model* model = rm.GetModel( modelIx );

	std::stringstream name;
	name << "_plane" << modelIx;

	model->name = name.str();
	model->vb = vb;
	model->ib = ib;
	model->vbOffset = rm.GetVbOffset( vb );
	model->ibOffset = rm.GetIbOffset( ib );

	vec2d gridSize = Divide( size, vec2d( cellCnt[0], cellCnt[1] ) );

	const uint32_t verticesPerQuad = 6;

	const uint32_t firstIndex = model->vbOffset;
	uint32_t indicesCnt = model->ibOffset;
	uint32_t vbIx = model->vbOffset;

	for ( int32_t j = 0; j <= cellCnt[ 1 ]; ++j )
	{
		for ( int32_t i = 0; i <= cellCnt[ 0 ]; ++i )
		{
			vertex_t v;

			v.pos = vec4d( i * gridSize[ 0 ] - 0.5 * size[ 0 ], j * gridSize[ 1 ] - 0.5 * size[ 1 ], 0.0, 1.0 );
			v.color = Color::White;
			v.normal = vec3d( 0.0, 0.0, 1.0 );

			rm.AddVertex( vb, v );
		}
	}
	model->vbEnd = rm.GetVbOffset( vb );

	for ( int32_t j = 0; j < cellCnt[ 1 ]; ++j )
	{
		for ( int32_t i = 0; i < cellCnt[ 0 ]; ++i )
		{
			uint32_t vIx[ 4 ];
			vIx[ 0 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 0 ) * ( cellCnt[ 1 ] + 1 ) );
			vIx[ 1 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 0 ) * ( cellCnt[ 1 ] + 1 ) );
			vIx[ 2 ] = static_cast<uint32_t>( firstIndex + ( i + 0 ) + ( j + 1 ) * ( cellCnt[ 1 ] + 1 ) );
			vIx[ 3 ] = static_cast<uint32_t>( firstIndex + ( i + 1 ) + ( j + 1 ) * ( cellCnt[ 1 ] + 1 ) );

			// Clockwise-winding
			rm.AddIndex( ib, vIx[ 0 ] );
			rm.AddIndex( ib, vIx[ 1 ] );
			rm.AddIndex( ib, vIx[ 2 ] );

			rm.AddIndex( ib, vIx[ 2 ] );
			rm.AddIndex( ib, vIx[ 1 ] );
			rm.AddIndex( ib, vIx[ 3 ] );
		}
	}
	model->ibEnd = rm.GetIbOffset( ib );

	model->material = material;

	return modelIx;
}


void CreateModelInstance( const uint32_t modelIx, const mat4x4d& modelMatrix, const bool smoothNormals, const Color& tint, ModelInstance* outInstance )
{
	const Model* model = rm.GetModel( modelIx );

	outInstance->transform = modelMatrix;
	const uint32_t vb = rm.AllocVB();
	outInstance->modelIx = modelIx;
	outInstance->triList.reserve( ( model->ibEnd - model->ibOffset ) / 3 );

	const uint32_t vbOffset = 0;

	using triIndices = std::tuple<uint32_t, uint32_t, uint32_t>;
	std::map< uint32_t, std::deque<triIndices> > vertToPolyMap;

	vec3d centroid = vec3d( 0.0, 0.0, 0.0 );
	for ( uint32_t i = model->vbOffset; i < model->vbEnd; ++i )
	{
		vertex_t vertex = *rm.GetVertex( model->vb, i );

		vertex.pos = outInstance->transform * vertex.pos;
		vertex.color *= tint;
		rm.AddVertex( vb, vertex );

		centroid += Trunc<4,1>( vertex.pos );
	}

	const uint32_t vbEnd = rm.GetVbOffset( vb );
	outInstance->centroid = centroid / (double)( vbEnd - vbOffset );

	for ( uint32_t i = model->ibOffset; i < model->ibEnd; i += 3 )
	{
		uint32_t indices[ 3 ];
		for ( uint32_t t = 0; t < 3; ++t )
		{
			indices[ t ] = rm.GetIndex( model->ib, i + t ) - model->vbOffset;
		}

		triIndices tIndices = std::make_tuple( indices[ 0 ], indices[ 1 ], indices[ 2 ] );

		vertToPolyMap[ indices[ 0 ] ].push_back( tIndices );
		vertToPolyMap[ indices[ 1 ] ].push_back( tIndices );
		vertToPolyMap[ indices[ 2 ] ].push_back( tIndices );
	}

	if ( smoothNormals )
	{
		using vertMapIter = std::map< uint32_t, std::deque<triIndices> >::iterator;

		for ( vertMapIter iter = vertToPolyMap.begin(); iter != vertToPolyMap.end(); ++iter )
		{
			vec3d interpretedNormal = vec3d( 0.0, 0.0, 0.0 );

			for ( std::deque<triIndices>::iterator polyListIter = iter->second.begin(); polyListIter != iter->second.end(); ++polyListIter )
			{
				const uint32_t i0 = std::get<0>( *polyListIter );
				const uint32_t i1 = std::get<1>( *polyListIter );
				const uint32_t i2 = std::get<2>( *polyListIter );

				vec3d pt0 = Trunc<4, 1>( rm.GetVertex( vb, i0 )->pos );
				vec3d pt1 = Trunc<4, 1>( rm.GetVertex( vb, i1 )->pos );
				vec3d pt2 = Trunc<4, 1>( rm.GetVertex( vb, i2 )->pos );

				vec3d b0 = ( pt1 - pt0 );
				vec3d b1 = ( pt2 - pt0 );

				interpretedNormal += Cross( b0, b1 ).Normalize();
			}

			vertex_t* vertex = rm.GetVertex( vb, iter->first );

			interpretedNormal.FlushDenorms();
			vertex->normal = interpretedNormal.Normalize();
			// vertex->normal = vertex->normal.Reverse();
		}
	}

	for ( uint32_t i = model->ibOffset; i < model->ibEnd; i += 3 )
	{
		uint32_t indices[ 3 ];
		for ( uint32_t t = 0; t < 3; ++t )
		{
			indices[ t ] = rm.GetIndex( model->ib, i + t ) - model->vbOffset;
		}

		vertex_t& v0 = *rm.GetVertex( vb, indices[ 0 ] );
		vertex_t& v1 = *rm.GetVertex( vb, indices[ 1 ] );
		vertex_t& v2 = *rm.GetVertex( vb, indices[ 2 ] );

		outInstance->triList.push_back( Triangle( v0, v1, v2 ) );
	}

	outInstance->material = model->material;

	outInstance->ComputeAABB();
}


mat4x4d GetModelToWorldAxis( const axisMode_t mode )
{
	// World Space: RHS Z+
	switch ( mode )
	{
	default:
	case RHS_XZY:
	return CreateMatrix4x4(	0.0, 0.0, -1.0, 0.0,
							-1.0, 0.0, 0.0, 0.0,
							0.0, 1.0, 0.0, 0.0,
							0.0, 0.0, 0.0, 1.0 );
	case RHS_XYZ:
		return CreateMatrix4x4( 1.0, 0.0, 0.0, 0.0,
								0.0, 1.0, 0.0, 0.0,
								0.0, 0.0, 1.0, 0.0,
								0.0, 0.0, 0.0, 1.0 );
	}
}


mat4x4d BuildModelMatrix( const vec3d& origin, const vec3d& degressZYZ, const double scale, const axisMode_t mode )
{
	const mat4x4d axis = GetModelToWorldAxis( mode );

	mat4x4d modelMatrix = axis;
	modelMatrix = ComputeScale( vec3d( scale ) ) * modelMatrix;
	modelMatrix = ComputeRotationZ( degressZYZ[ 2 ] ) * modelMatrix;
	modelMatrix = ComputeRotationY( degressZYZ[ 1 ] ) * modelMatrix;
	modelMatrix = ComputeRotationX( degressZYZ[ 0 ] ) * modelMatrix;
	SetTranslation( modelMatrix, origin );

	return modelMatrix;
}


void BuildScene()
{
	uint32_t modelIx;
	uint32_t vb = rm.AllocVB();
	uint32_t ib = rm.AllocIB();

	modelIx = LoadModel( std::string( "models/teapot.off" ), vb, ib );
	if ( modelIx >= 0 )
	{
		mat4x4d modelMatrix;

		ModelInstance teapot0;
		modelMatrix = BuildModelMatrix( vec3d( 30.0, 120.0, 0.0 ), vec3d( 0.0, 0.0, -90.0 ), 1.0, RHS_XZY );
		CreateModelInstance( modelIx, modelMatrix, true, Color::Yellow, &teapot0 );
		scene.models.push_back( teapot0 );

		ModelInstance teapot1;
		modelMatrix = BuildModelMatrix( vec3d( -30.0, -50.0, 0.0 ), vec3d( 0.0, 0.0, 0.0 ), 1.0, RHS_XZY );
		CreateModelInstance( modelIx, modelMatrix, true, Color::Green, &teapot1 );
		scene.models.push_back( teapot1 );
	}

	modelIx = LoadModel( std::string( "models/sphere.off" ), vb, ib );
	if( modelIx >= 0 )
	{
		mat4x4d modelMatrix;

		ModelInstance sphere0;
		modelMatrix = BuildModelMatrix( vec3d( 30.0, 40.0, 0.0 ), vec3d( 0.0, 0.0, -30.0 ), 0.8, RHS_XZY );
		CreateModelInstance( modelIx, modelMatrix, true, Color::Red, &sphere0 );
		scene.models.push_back( sphere0 );

		ModelInstance sphere1;
		modelMatrix = BuildModelMatrix( vec3d( -50.0, -10.0, 10.0 ), vec3d( 0.0, 0.0, 0.0 ), .3, RHS_XZY );
		CreateModelInstance( modelIx, modelMatrix, true, Color::Blue, &sphere1 );
		scene.models.push_back( sphere1 );
	}

	modelIx = CreatePlaneModel( vb, ib, vec2d( 500.0 ), vec2i( 1 ), mirrorMaterial );
	if ( modelIx >= 0 )
	{
		mat4x4d modelMatrix;

		ModelInstance plane0;
		modelMatrix = BuildModelMatrix( vec3d( 0.0, 0.0, -30.0 ), vec3d( 0.0, 0.0, 0.0 ), 1.0, RHS_XYZ );
		CreateModelInstance( modelIx, modelMatrix, false, Color::White, &plane0 );
		scene.models.push_back( plane0 );
	}

	scene.lights.reserve( 3 );
	{
		light_t l;
		l.pos = vec3d( -200.0, -100.0, 50.0 );
		l.intensity = 1.0f;
		scene.lights.push_back( l );
		/*
		l.pos = vec3d( 150, 20.0, -500.0 );
		l.intensity = 0.1f;
		scene.lights.push_back( l );

		l.pos = vec3d( 20.0, 150.0, -25.0 );
		l.intensity = 0.2f;
		scene.lights.push_back( l );
		*/
	}

	const size_t modelCnt = scene.models.size();
	for ( size_t m = 0; m < modelCnt; ++m )
	{
		ModelInstance& model = scene.models[ m ];
		model.ComputeAABB();
		scene.aabb.Expand( model.aabb.min );
		scene.aabb.Expand( model.aabb.max );
	}
}


void DrawGradientImage( Bitmap& bitmap, const Color& color0, const Color& color1, const float power = 1.0f )
{
	for ( uint32_t j = 0; j < bitmap.GetHeight(); ++j )
	{
		const float t = pow( j / static_cast<float>( bitmap.GetHeight() ), power );

		const uint32_t gradient = LinearToSrgb( Lerp( color0, color1, t ) ).AsR8G8B8A8();
		for ( uint32_t i = 0; i < bitmap.GetWidth(); ++i )
		{
			bitmap.SetPixel( i, j, gradient );
		}
	}
}


int main(void)
{
	std::cout << "Running Raytracer" << std::endl;

	BuildScene();

	Bitmap diffuse = Bitmap( RenderWidth, RenderHeight, Color::Red );
	dbg.diffuse = &diffuse;

	Bitmap normal = Bitmap( RenderWidth, RenderHeight, Color::White );
	dbg.normal = &normal;

	Bitmap wireframe = Bitmap( RenderWidth, RenderHeight, Color::LGrey );
	dbg.wireframe = &wireframe;

	Bitmap topWireframe = Bitmap( RenderWidth, RenderHeight, Color::LGrey );
	dbg.topWire = &topWireframe;

	Bitmap sizeWireframe = Bitmap( RenderWidth, RenderHeight, Color::LGrey );
	dbg.sideWire = &sizeWireframe;

	Bitmap image = Bitmap( RenderWidth, RenderHeight, Color::DGrey );
	DrawGradientImage( image, Color::Blue, Color::Red, 0.8f );

	const int32_t imageCnt = 1;
	for ( int32_t i = 0; i < imageCnt; ++i )
	{
		DrawScene( image );

		std::stringstream ss;
		ss << "output/out" << i << ".bmp";

		image.Write( ss.str() );
		std::cout << ss.str() << std::endl;

		std::stringstream dbgDiffuse;
		dbgDiffuse << "output/dbgDiffuse" << i << ".bmp";
		dbg.diffuse->Write( dbgDiffuse.str() );

		std::stringstream dbgNormal;
		dbgNormal << "output/dbgNormal" << i << ".bmp";
		dbg.normal->Write( dbgNormal.str() );

		std::stringstream dbgWireframe;
		dbgWireframe << "output/dbgWireframe" << i << ".bmp";
		dbg.wireframe->Write( dbgWireframe.str() );

		std::stringstream dbgTopWireframe;
		dbgTopWireframe << "output/dbgTop" << i << ".bmp";
		dbg.topWire->Write( dbgTopWireframe.str() );

		std::stringstream dbgSideWireframe;
		dbgSideWireframe << "output/dbgSide" << i << ".bmp";
		dbg.sideWire->Write( dbgSideWireframe.str() );
	}

	std::cout << "Raytrace Finished." << std::endl;
	return 1;
}