
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
#include "common.h"
#include "bitmap.h"
#include "color.h"
#include "mathVector.h"
#include "matrix.h"
#include "meshIO.h"
#include "rasterLib.h"
#include "geom.h"
#include "resourceManager.h"
#include "camera.h"
#include "scene.h"
#include "debug.h"
#include "globals.h"

ResourceManager	rm;

material_t mirrorMaterial = { 1.0, 0.5, 0.5, 1.0, 1.0 };

Scene			scene;
SceneView		views[4];
debug_t			dbg;
Bitmap*			colorBuffer;
Bitmap*			depthBuffer;

void RasterScene( Bitmap& bitmap, const SceneView& view, bool wireFrame = true );

sample_t RecordSurfaceInfo( const Ray& r, const double t, const uint32_t triIndex, const uint32_t modelIx )
{
	const std::vector<Triangle>& triList = scene.models[ modelIx ].triList;
	const Triangle& tri = triList[ triIndex ];

	sample_t sample;

	sample.pt = r.GetPoint( t );
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
	sample.color = Vec4dToColor( mixedColor );

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

		vec3d viewVector = ray.GetVector().Reverse();
		viewVector = viewVector.Normalize();

		const size_t lightCnt = scene.lights.size();
		for ( size_t li = 0; li < lightCnt; ++li )
		{
			vec3d& lightPos = scene.lights[ li ].pos;

			Ray shadowRay = Ray( surfaceSample.pt, lightPos );

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

				vec3d lightDir = shadowRay.GetVector();
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
			double t = 0.0;

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
				t += sample.t;
				coverage += sample.hitCode != HIT_NONE ? 1.0 : 0.0;
			}
		
			if ( coverage > 0.0 )
			{
				int32_t imageX = static_cast<int32_t>( px );
				int32_t imageY = static_cast<int32_t>( RenderHeight - py - 1 );

				coverage /= subSampleCnt;
				diffuse /= subSampleCnt;
				normal = normal.Normalize();
				t /= subSampleCnt;

				Color src = Color( LinearToSrgb( ( 1.0f / subSampleCnt ) * pixelColor ) );
				src.rgba().a = (float) coverage;

				// normal = normal.Reverse();
				Color normColor = Vec4dToColor( vec4d( normal, 1.0 ) );
				dbg.diffuse->SetPixel( imageX, imageY, Color( (float)-diffuse ).AsR8G8B8A8() );
				dbg.normal->SetPixel( imageX, imageY, normColor.AsR8G8B8A8() );

				Color dest = Color( bitmap.GetPixel( imageX, imageY ) );

				Pixel pixel = BlendColor( src, dest, blendMode_t::SRCALPHA ).AsR8G8B8A8();
				bitmap.SetPixel( imageX, imageY, pixel.r8g8b8a8 );
			}
		}
		std::cout << static_cast<int>( 100.0 * ( py / (double) bitmap.GetHeight() ) ) << "% ";
	}
#endif

	RasterScene( *colorBuffer, views[ VIEW_CAMERA ], false );
	RasterScene( *dbg.wireframe, views[ VIEW_CAMERA ] );
	RasterScene( *dbg.topWire, views[ VIEW_TOP ] );
	RasterScene( *dbg.sideWire, views[ VIEW_SIDE ] );
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

	modelIx = LoadModelObj( std::string( "models/teapot.obj" ), vb, ib );

	{
	//	StoreModelObj( std::string( "models/teapotout.obj" ), modelIx );
	}

	//modelIx = LoadModel( std::string( "models/teapot.off" ), vb, ib );
	if ( modelIx >= 0 )
	{
		mat4x4d modelMatrix;
		
		ModelInstance teapot0;
		modelMatrix = BuildModelMatrix( vec3d( 30.0, 120.0, 0.0 ), vec3d( 0.0, 0.0, -90.0 ), 1.0, RHS_XZY );
		CreateModelInstance( modelIx, modelMatrix, true, Color::Yellow, &teapot0 );
		scene.models.push_back( teapot0 );

		ModelInstance teapot1;
		modelMatrix = BuildModelMatrix( vec3d( -30.0, -50.0, 0.0 ), vec3d( 0.0, 0.0, 30.0 ), 1.0, RHS_XZY );
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

	Bitmap cb = Bitmap( RenderWidth, RenderHeight, Color::Black );
	colorBuffer = &cb;

	Bitmap db = Bitmap( RenderWidth, RenderHeight, Color::Black );
	depthBuffer = &db;

	Bitmap image = Bitmap( RenderWidth, RenderHeight, Color::DGrey );
	DrawGradientImage( image, Color::Blue, Color::Red, 0.8f );

	const int32_t imageCnt = 1;
	for ( int32_t i = 0; i < imageCnt; ++i )
	{
		DrawScene( image );

		std::stringstream ss;

#if USE_RAYTRACE
		ss << "output/out" << i << ".bmp";

		image.Write( ss.str() );
		std::cout << ss.str() << std::endl;

		std::stringstream dbgDiffuse;
		dbgDiffuse << "output/dbgDiffuse" << i << ".bmp";
		dbg.diffuse->Write( dbgDiffuse.str() );

		std::stringstream dbgNormal;
		dbgNormal << "output/dbgNormal" << i << ".bmp";
		dbg.normal->Write( dbgNormal.str() );
#endif

		std::stringstream colorBufferName;
		colorBufferName << "output/colorBuffer" << i << ".bmp";
		colorBuffer->Write( colorBufferName.str() );

		std::stringstream depthBufferName;
		depthBufferName << "output/depthBuffer" << i << ".bmp";
		depthBuffer->Write( depthBufferName.str() );

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