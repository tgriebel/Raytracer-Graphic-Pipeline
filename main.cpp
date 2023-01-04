
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
#include <image/color.h>
#include <math/mathVector.h>
#include <math/matrix.h>
#include <io/meshIO.h>
#include <primitives/geom.h>
#include <scene/resourceManager.h>
#include <scene/camera.h>
#include <image/image.h>
#include <core/rasterLib.h>
#include <core/util.h>
#include "scene.h"
#include "debug.h"
#include "globals.h"
#include "timer.h"
#include "raytrace.h"

ResourceManager	rm;

matHdl_t		colorMaterialId = 16;
matHdl_t		diffuseMaterialId = 17;
matHdl_t		mirrorMaterialId = 18;

RtScene			scene;
SceneView		views[4];
debug_t			dbg;
Image<Color>	colorBuffer;
Image<float>	depthBuffer;

extern Image<float> zBuffer;

void RasterScene( Image<Color>& image, const SceneView& view, bool wireFrame = true );

void ImageToBitmap( const Image<Color>& image, Bitmap& bitmap );
void ImageToBitmap( const Image<float>& image, Bitmap& bitmap );
void BitmapToImage( const Bitmap& bitmap, Image<Color>& image );

SceneView SetupFrontView()
{
	SceneView view;

	view.targetSize = RenderSize;
	view.camera.Init(	vec4f( -280.0f, -30.0f, 50.0f, 0.0f ),
						CreateMatrix4x4( 0.0f, -1.0f, 0.0f, 0.0f,
										 0.0f, 0.0f, -1.0f, 0.0f,
										 -1.0f, 0.0f, 0.0f, 0.0f,
										 0.0f, 0.0f, 0.0f, 1.0f ),
						AspectRatio( view.targetSize ),
						CameraFov,
						CameraNearPlane,
						CameraFarPlane );

	view.viewTransform = view.camera.GetViewMatrix().Transpose();
	view.projTransform = view.camera.GetPerspectiveMatrix().Transpose();
	view.projView = view.projTransform * view.viewTransform;

	return view;
}


SceneView SetupTopView()
{
	SceneView view;

	view.targetSize = RenderSize;
	view.camera.Init(	vec4f( 0.0f, 0.0f, 280.0f, 0.0f ),
						CreateMatrix4x4( 0.0f, -1.0f, 0.0f, 0.0f,
										 -1.0f, 0.0f, 0.0f, 0.0f,
										 0.0f, 0.0f, 1.0f, 0.0f,
										 0.0f, 0.0f, 0.0f, 1.0f ),
						AspectRatio( view.targetSize ),
						CameraFov,
						CameraNearPlane,
						CameraFarPlane );

	view.viewTransform = view.camera.GetViewMatrix().Transpose();
	view.projTransform = view.camera.GetPerspectiveMatrix().Transpose();
	view.projView = view.projTransform * view.viewTransform;

	return view;
}


SceneView SetupSideView()
{
	SceneView view;

	view.targetSize = RenderSize;
	view.camera.Init(	vec4f( 0.0f, 280.0f, 0.0f, 0.0f ),
						CreateMatrix4x4( -1.0f, 0.0f, 0.0f, 0.0f,
										 0.0f, 0.0f, -1.0f, 0.0f,
										 0.0f, 1.0f, 0.0f, 0.0f,
										 0.0f, 1.0f, 0.0f, 1.0f ),
						AspectRatio( view.targetSize ),
						CameraFov,
						CameraNearPlane,
						CameraFarPlane );

	view.viewTransform = view.camera.GetViewMatrix().Transpose();
	view.projTransform = view.camera.GetPerspectiveMatrix().Transpose();
	view.projView = view.projTransform * view.viewTransform;

	return view;
}


void SetupViews()
{
	views[ VIEW_CAMERA ] = SetupFrontView();
	views[ VIEW_FRONT ] = SetupFrontView();
	views[ VIEW_TOP ] = SetupTopView();
	views[ VIEW_SIDE ] = SetupSideView();
}


void TraceScene( const SceneView& view, Image<Color>& image )
{
#if USE_RAYTRACE

	uint32_t threadsLaunched = 0;
	uint32_t threadsComplete = 0;
	std::vector<std::thread> threads;

	uint32_t renderWidth = view.targetSize[ 0 ];
	uint32_t renderHeight = view.targetSize[ 1 ];

	const uint32_t patchSize = 120;
	for ( uint32_t py = 0; py < renderHeight; py += patchSize )
	{
		for ( uint32_t px = 0; px < renderWidth; px += patchSize )
		{
			vec2i patch;
			patch[ 0 ] = Clamp( px + patchSize, px, renderWidth );
			patch[ 1 ] = Clamp( py + patchSize, py, renderHeight );

			threads.push_back( std::thread( TracePatch, view, &image, vec2i( px, py ), patch ) );
			++threadsLaunched;
		}
	}

	for ( auto& thread : threads )
	{
		thread.join(); // TODO: replace with non-blocking call for better messaging
		++threadsComplete;
		std::cout << static_cast<int>( 100.0 * ( threadsComplete / (float)threadsLaunched ) ) << "% ";
	}
#endif
}


void RastizeViews()
{
#if USE_RASTERIZE
	RasterScene( colorBuffer, views[ VIEW_FRONT ], false );
#endif

#if DRAW_WIREFRAME
	RasterScene( dbg.wireframe, views[ VIEW_FRONT ] );
	RasterScene( dbg.topWire, views[ VIEW_TOP ] );
	RasterScene( dbg.sideWire, views[ VIEW_SIDE ] );
#endif
}


mat4x4f GetModelToWorldAxis( const axisMode_t mode )
{
	// World Space: RHS Z+
	switch ( mode )
	{
	default:
	case RHS_XZY:
	return CreateMatrix4x4(	0.0f, 0.0f, -1.0f, 0.0f,
							-1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f );
	case RHS_XYZ:
		return CreateMatrix4x4( 1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f );
	}
}


mat4x4f BuildModelMatrix( const vec3f& origin, const vec3f& degressZYZ, const float scale, const axisMode_t mode )
{
	const mat4x4f axis = GetModelToWorldAxis( mode );

	mat4x4f modelMatrix = axis;
	modelMatrix = ComputeScale( vec3f( scale ) ) * modelMatrix;
	modelMatrix = ComputeRotationZ( degressZYZ[ 2 ] ) * modelMatrix;
	modelMatrix = ComputeRotationY( degressZYZ[ 1 ] ) * modelMatrix;
	modelMatrix = ComputeRotationX( degressZYZ[ 0 ] ) * modelMatrix;
	SetTranslation( modelMatrix, origin );

	return modelMatrix;
}


void CreateMaterials( ResourceManager& rm )
{
	for( uint32_t i = 0; i < 16; ++i )
	{
		material_t dbgMaterial;
		memset( &dbgMaterial, 0, sizeof( material_t ) );
		dbgMaterial.Ka = Color( 1.0f ).AsRGBf();
		dbgMaterial.Kd = Color( DbgColors[ i ] ).AsRGBf();
		dbgMaterial.Ks = Color( 0.0f ).AsRGBf();
		dbgMaterial.Ke = Color( 0.0f ).AsRGBf();
		dbgMaterial.Tr = 0.0f;
		rm.StoreMaterialCopy( dbgMaterial );
	}

	material_t colorMaterial;
	memset( &colorMaterial, 0, sizeof( material_t ) );
	colorMaterial.Ka = Color( 1.0f ).AsRGBf();
	colorMaterial.Kd = Color( 1.0f ).AsRGBf();
	colorMaterial.Ks = Color( 0.0f ).AsRGBf();
	colorMaterial.Ke = Color( 0.0f ).AsRGBf();
	colorMaterial.Tr = 0.0f;
	rm.StoreMaterialCopy( colorMaterial );

	material_t diffuseMaterial;
	memset( &diffuseMaterial, 0, sizeof( material_t ) );
	diffuseMaterial.Ka = Color( 1.0f ).AsRGBf();
	diffuseMaterial.Kd = Color( 1.0f ).AsRGBf();
	diffuseMaterial.Ks = Color( 1.0f ).AsRGBf();
	diffuseMaterial.Ke = Color( 1.0f ).AsRGBf();
	diffuseMaterial.Tr = 0.0f;
	rm.StoreMaterialCopy( diffuseMaterial );

	material_t mirrorMaterial;
	memset( &mirrorMaterial, 0, sizeof( material_t ) );
	mirrorMaterial.Ka = Color( 1.0f ).AsRGBf();
	mirrorMaterial.Kd = Color( 1.0f ).AsRGBf();
	mirrorMaterial.Ks = Color( 1.0f ).AsRGBf();
	mirrorMaterial.Ke = Color( 1.0f ).AsRGBf();
	mirrorMaterial.Tr = 0.8f;
	rm.StoreMaterialCopy( mirrorMaterial );
}


void BuildScene()
{
	uint32_t modelIx;
	uint32_t vb = rm.AllocVB();
	uint32_t ib = rm.AllocIB();

	/*
	modelIx = LoadModelObj( std::string( "models/teapot.obj" ), vb, ib );
	if ( modelIx >= 0 )
	{
		mat4x4f modelMatrix;
		
		RtModel teapot0;
		modelMatrix = BuildModelMatrix( vec3f( 30.0, 120.0, 10.0 ), vec3f( 0.0, 0.0, -90.0 ), 1.0, RHS_XZY );
		CreateModelInstance( modelIx, modelMatrix, true, Color::Yellow, &teapot0, colorMaterialId );
		scene.models.push_back( teapot0 );
		
		RtModel teapot1;
		modelMatrix = BuildModelMatrix( vec3f( -30.0, -50.0, 10.0 ), vec3f( 0.0, 0.0, 30.0 ), 1.0, RHS_XZY );
		CreateModelInstance( modelIx, modelMatrix, true, Color::Green, &teapot1, colorMaterialId );
		scene.models.push_back( teapot1 );
	}
	*/

	rm.PushVB( vb );
	rm.PushIB( ib );

	modelIx = LoadModelObj( std::string( "models/sphere.obj" ), rm );
	if( modelIx >= 0 )
	{
		mat4x4f modelMatrix;

		RtModel sphere0;
		modelMatrix = BuildModelMatrix( vec3f( 30.0f, -70.0f, 0.0f ), vec3f( 0.0f, 0.0f, 0.0f ), 0.5f, RHS_XZY );
		CreateRayTraceModel( rm, modelIx, modelMatrix, true, Color::White, &sphere0, mirrorMaterialId );
		scene.models.push_back( sphere0 );

		RtModel sphere1;
		modelMatrix = BuildModelMatrix( vec3f( 30.0f, -20.0f, 0.0f ), vec3f( 0.0f, 0.0f, 0.0f ), 0.5f, RHS_XZY );
		CreateRayTraceModel( rm, modelIx, modelMatrix, true, Color::Red, &sphere1 );
		scene.models.push_back( sphere1 );

		RtModel sphere2;
		modelMatrix = BuildModelMatrix( vec3f( 30.0f, 30.0f, 0.0f ), vec3f( 0.0f, 0.0f, 0.0f ), 0.5f, RHS_XZY );
		CreateRayTraceModel( rm, modelIx, modelMatrix, true, Color::White, &sphere2, mirrorMaterialId );
		scene.models.push_back( sphere2 );

		RtModel sphere3;
		modelMatrix = BuildModelMatrix( vec3f( 30.0f, 80.0f, 0.0f ), vec3f( 0.0f, 0.0f, 0.0f ), 0.5f, RHS_XZY );
		CreateRayTraceModel( rm, modelIx, modelMatrix, true, Color::White, &sphere3, mirrorMaterialId );
		scene.models.push_back( sphere3 );
	}

	/*
	modelIx = LoadModelBin( std::string( "models/12140_Skull_v3_L2.mdl" ), rm );
	if ( modelIx >= 0 )
	{
		mat4x4f modelMatrix;

		RtModel skull0;
		modelMatrix = BuildModelMatrix( vec3f( 30.0, 120.0, 10.0 ), vec3f( 0.0, 90.0, 40.0 ), 4.0, RHS_XZY );
		CreateModelInstance( rm, modelIx, modelMatrix, true, Color::Gold, &skull0 );
		scene.models.push_back( skull0 );

		RtModel skull1;
		modelMatrix = BuildModelMatrix( vec3f( -30.0, -120.0, -10.0 ), vec3f( 0.0, 90.0, 0.0 ), 5.0, RHS_XZY );
		CreateModelInstance( rm, modelIx, modelMatrix, true, Color::Gold, &skull1 );
		scene.models.push_back( skull1 );
	}
	*/

	/*
	modelIx = LoadModelBin( std::string( "models/911_scene.mdl" ), rm );
	if ( modelIx >= 0 )
	{
		mat4x4f modelMatrix;

		RtModel car0;
		modelMatrix = BuildModelMatrix( vec3f( -30.0, -100.0, -10.0 ), vec3f( 0.0, 0.0, 0.0 ), 30.0, RHS_XZY );
		CreateModelInstance( rm, modelIx, modelMatrix, true, Color::Red, &car0 );
		scene.models.push_back( car0 );
	}
	*/

	modelIx = CreatePlaneModel( rm, vec2f( 500.0f ), vec2i( 1 ), colorMaterialId );
	if ( modelIx >= 0 )
	{
		mat4x4f modelMatrix;

		RtModel plane0;
		modelMatrix = BuildModelMatrix( vec3f( 0.0f, 0.0f, -10.0f ), vec3f( 0.0f, 0.0f, 0.0f ), 1.0f, RHS_XYZ );
		CreateRayTraceModel( rm, modelIx, modelMatrix, false, Color::DGrey, &plane0, colorMaterialId );
		scene.models.push_back( plane0 );
	}

	scene.lights.reserve( 3 );
	{
		light_t l;
		l.pos = vec3f( -200.0f, -100.0f, 50.0f );
		l.intensity = vec3f( 1.0f, 1.0f, 1.0f );
		scene.lights.push_back( l );
		/*
		l.pos = vec3f( 150, 20.0, 0.0 );
		l.intensity = vec3f( 0.0, 1.0, 0.0 );
		scene.lights.push_back( l );

		l.pos = vec3f( 20.0, 150.0, 25.0 );
		l.intensity = vec3f( 0.0, 0.0, 1.0 );
		scene.lights.push_back( l );
		*/
	}

	const size_t modelCnt = scene.models.size();
	for ( size_t m = 0; m < modelCnt; ++m )
	{
		RtModel& model = scene.models[ m ];
		scene.aabb.Expand( model.octree.GetAABB().min );
		scene.aabb.Expand( model.octree.GetAABB().max );
	}
}


void DrawGradientImage( Image<Color>& image, const Color& color0, const Color& color1, const float power = 1.0f )
{
	for ( uint32_t j = 0; j < image.GetHeight(); ++j )
	{
		const float t = pow( j / static_cast<float>( image.GetHeight() ), power );

		const uint32_t gradient = LinearToSrgb( Lerp( color0, color1, t ) ).AsR8G8B8A8();
		for ( uint32_t i = 0; i < image.GetWidth(); ++i )
		{
			image.SetPixel( i, j, gradient );
		}
	}
}

template<typename T>
void WriteImage( const Image<T>& image, const std::string& path, const int32_t number = -1 )
{
	std::stringstream ss;

	ss << path << "/";
	
	const char* name = image.GetName();
	if( ( name == nullptr ) || ( name == "" ) )
	{
		ss << reinterpret_cast<uint64_t>( &image );
	}
	else
	{
		ss << name;
	}
	
	if( number >= 0 )
	{
		ss << "_" << number;
	}

	ss << ".bmp";

	Bitmap bitmap = Bitmap( image.GetWidth(), image.GetHeight() );
	ImageToBitmap( image, bitmap );
	bitmap.Write( ss.str() );
}


int main(void)
{
	std::cout << "Running Raytracer/Rasterizer" << std::endl;

	Timer loadTimer;

	CreateMaterials( rm );

	loadTimer.Start();
	BuildScene();
	loadTimer.Stop();

	std::cout << "Load Time: " << loadTimer.GetElapsed() << "ms" << std::endl;

	dbg.diffuse = Image<Color>( RenderWidth, RenderHeight, Color::Red, "dbgDiffuse" );
	dbg.normal = Image<Color>( RenderWidth, RenderHeight, Color::White, "dbgNormal" );
	dbg.wireframe = Image<Color>( RenderWidth, RenderHeight, Color::LGrey, "dbgWireframe" );
	dbg.topWire = Image<Color>( RenderWidth, RenderHeight, Color::LGrey, "dbgTopWire" );
	dbg.sideWire = Image<Color>( RenderWidth, RenderHeight, Color::LGrey, "dbgSideWire" );

	colorBuffer = Image<Color>( RenderWidth, RenderHeight, Color::Black, "colorBuffer" );
	depthBuffer = Image<float>( RenderWidth, RenderHeight, 0.0f, "depthBuffer" );

	Image<Color> frameBuffer = Image<Color>( RenderWidth, RenderHeight, Color::DGrey, "_frameBuffer" );
	DrawGradientImage( frameBuffer, Color::Blue, Color::Red, 0.8f );

	SetupViews();

	const int32_t imageCnt = 1;
	for ( int32_t i = 0; i < imageCnt; ++i )
	{
		Timer traceTimer;

		traceTimer.Start();
		TraceScene( views[ VIEW_CAMERA ], frameBuffer );
		traceTimer.Stop();

		RastizeViews();

		std::cout << "\n\nTrace Time: " << traceTimer.GetElapsed() << "ms" << std::endl;

		WriteImage( frameBuffer, "output", i );
	}

	WriteImage( dbg.diffuse, "output" );
	WriteImage( dbg.normal, "output" );

	WriteImage( colorBuffer, "output" );
	WriteImage( depthBuffer, "output" );

	WriteImage( dbg.wireframe, "output" );
	WriteImage( dbg.topWire, "output" );
	WriteImage( dbg.sideWire, "output" );

	WriteImage( zBuffer, "output" );

	std::cout << "Raytrace Finished." << std::endl;
	return 1;
}