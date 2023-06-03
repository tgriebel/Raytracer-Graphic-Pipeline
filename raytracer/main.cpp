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
#include <gfxcore/image/bitmap.h>
#include <gfxcore/image/color.h>
#include <gfxcore/math/vector.h>
#include <gfxcore/math/matrix.h>
#include <gfxcore/io/meshIO.h>
#include <gfxcore/primitives/geom.h>
#include <gfxcore/scene/resourceManager.h>
#include <gfxcore/scene/assetManager.h>
#include <gfxcore/scene/camera.h>
#include <gfxcore/image/image.h>
#include <gfxcore/core/rasterLib.h>
#include <gfxcore/core/util.h>
#include <gfxcore/io/io.h>
#include "scene.h"
#include <timer.h>
#include "debug.h"
#include "globals.h"
#include "raytrace.h"

ResourceManager	rm;

matHdl_t		colorMaterialId = 16;
matHdl_t		diffuseMaterialId = 17;
matHdl_t		mirrorMaterialId = 18;

RtView				rtViews[4];
debug_t				dbg;
ImageBuffer<Color>	colorBuffer;
ImageBuffer<float>	depthBuffer;

extern ImageBuffer<float> zBuffer;

void RasterScene( ImageBuffer<Color>& image, const RtView& view, bool wireFrame = true );

void ImageToBitmap( const ImageBuffer<Color>& image, Bitmap& bitmap );
void ImageToBitmap( const ImageBuffer<float>& image, Bitmap& bitmap );
void BitmapToImage( const Bitmap& bitmap, ImageBuffer<Color>& image );

RtView SetupFrontView()
{
	RtView view;

	view.targetSize = RenderSize;
	view.camera.Init(	vec4f( -100.0f, -70.0f, 0.0f, 0.0f ),
						CreateMatrix4x4( 0.0f, -1.0f, 0.0f, 0.0f,
										 0.0f, 0.0f, -1.0f, 0.0f,
										 -1.0f, 0.0f, 0.0f, 0.0f,
										 0.0f, 0.0f, 0.0f, 1.0f ),
						AspectRatio( view.targetSize ),
						CameraFov,
						CameraNearPlane,
						CameraFarPlane );

	view.viewTransform = view.camera.GetViewMatrix();
	view.projTransform = view.camera.GetPerspectiveMatrix();
	view.projView = view.projTransform * view.viewTransform;

	return view;
}


RtView SetupTopView()
{
	RtView view;

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

	view.viewTransform = view.camera.GetViewMatrix();
	view.projTransform = view.camera.GetPerspectiveMatrix();
	view.projView = view.projTransform * view.viewTransform;

	return view;
}


RtView SetupSideView()
{
	RtView view;

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

	view.viewTransform = view.camera.GetViewMatrix();
	view.projTransform = view.camera.GetPerspectiveMatrix();
	view.projView = view.projTransform * view.viewTransform;

	return view;
}


void SetupViews()
{
	rtViews[ VIEW_CAMERA ] = SetupFrontView();
	rtViews[ VIEW_FRONT ] = SetupFrontView();
	rtViews[ VIEW_TOP ] = SetupTopView();
	rtViews[ VIEW_SIDE ] = SetupSideView();
}


void RasterizeViews( RtScene& rtScene )
{
#if USE_RASTERIZE
	RasterScene( colorBuffer, rtViews[ VIEW_FRONT ], rtScene, false );
#endif

#if DRAW_WIREFRAME
	RasterScene( dbg.wireframe, rtViews[ VIEW_FRONT ], rtScene );
	RasterScene( dbg.topWire, rtViews[ VIEW_TOP ], rtScene );
	RasterScene( dbg.sideWire, rtViews[ VIEW_SIDE ], rtScene );
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


void CreateMaterials( AssetManager& assets )
{
	for( uint32_t i = 0; i < 16; ++i )
	{
		Material dbgMaterial;
		dbgMaterial.Ka( Color( 1.0f ).AsRGBf() );
		dbgMaterial.Kd( Color( DbgColors[ i ] ).AsRGBf() );
		dbgMaterial.Ks( Color( 0.0f ).AsRGBf() );
		dbgMaterial.Ke( Color( 0.0f ).AsRGBf() );
		dbgMaterial.Tr( 0.0f );

		std::stringstream ss;
		ss << "default_" << i;
		assets.materialLib.Add( ss.str().c_str(), dbgMaterial );
	}

	Material colorMaterial;
	colorMaterial.Ka( Color( 1.0f ).AsRGBf() );
	colorMaterial.Kd( Color( 1.0f ).AsRGBf() );
	colorMaterial.Ks( Color( 0.0f ).AsRGBf() );
	colorMaterial.Ke( Color( 0.0f ).AsRGBf() );
	colorMaterial.Tr( 0.0f );
	assets.materialLib.Add( "colorMaterial", colorMaterial );

	Material diffuseMaterial;
	diffuseMaterial.Ka( Color( 1.0f ).AsRGBf() );
	diffuseMaterial.Kd( Color( 1.0f ).AsRGBf() );
	diffuseMaterial.Ks( Color( 1.0f ).AsRGBf() );
	diffuseMaterial.Ke( Color( 1.0f ).AsRGBf() );
	diffuseMaterial.Tr( 0.0f );
	assets.materialLib.Add( "diffuseMaterial", diffuseMaterial );

	Material mirrorMaterial;
	mirrorMaterial.Ka( Color( 1.0f ).AsRGBf() );
	mirrorMaterial.Kd( Color( 1.0f ).AsRGBf() );
	mirrorMaterial.Ks( Color( 1.0f ).AsRGBf() );
	mirrorMaterial.Ke( Color( 1.0f ).AsRGBf() );
	mirrorMaterial.Tr( 0.8f );
	assets.materialLib.Add( "mirrorMaterial", mirrorMaterial );
}


void BuildRtSceneView( AssetManager& assets, RtScene& rtScene )
{
	hdl_t modelHdl;
	Model model;
	if( LoadRawModel( assets, "pawn.obj", "models\\", "textures\\", model ) ) {
		modelHdl = assets.modelLib.Add( "sphere", model );
	}
	if( modelHdl != INVALID_HDL )
	{
		{
			Entity ent;
			ent.modelHdl = modelHdl;
			ent.SetScale( 50.0f );
			ent.SetRotation( vec3f( 0.0f, 0.0f, 180.0f ) );
			ent.SetOrigin( vec3f( 0.0f, -70.0f, 0.0f ) );		
			
			RtModel sphere;
			CreateRayTraceModel( assets, &ent, &sphere );
			rtScene.models.push_back( sphere );
		}
		/*
		{
			Entity ent;
			ent.modelHdl = modelHdl;
			ent.SetScale( vec3f( 40.5f ) );
			ent.SetRotation( vec3f( 0.0f, 0.0f, 0.0f ) );
			ent.SetOrigin( vec3f( 30.0f, -20.0f, 0.0f ) );

			RtModel sphere;
			CreateRayTraceModel( scene, &ent, &sphere );
			rtScene.models.push_back( sphere );
		}

		{
			Entity ent;
			ent.modelHdl = modelHdl;
			ent.SetScale( vec3f( 40.5f ) );
			ent.SetRotation( vec3f( 0.0f, 0.0f, 0.0f ) );
			ent.SetOrigin( vec3f( 30.0f, 30.0f, 0.0f ) );
			
			RtModel sphere;
			CreateRayTraceModel( scene, &ent, &sphere );
			rtScene.models.push_back( sphere );
		}

		{
			Entity ent;
			ent.modelHdl = modelHdl;
			ent.SetScale( vec3f( 40.5f ) );
			ent.SetRotation( vec3f( 0.0f, 0.0f, 0.0f ) );
			ent.SetOrigin( vec3f( 30.0f, 80.0f, 0.0f ) );
			
			RtModel sphere;
			CreateRayTraceModel( scene, &ent, &sphere );
			rtScene.models.push_back( sphere );
		}
		*/
	}

	rtScene.lights.reserve( 3 );
	{
		light_t l;
		l.lightPos = vec4f( -200.0f, -100.0f, 50.0f, 1.0f );
		l.intensity = vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
		rtScene.lights.push_back( l );
		/*
		l.pos = vec3f( 150, 20.0, 0.0 );
		l.intensity = vec3f( 0.0, 1.0, 0.0 );
		rtScene.lights.push_back( l );

		l.pos = vec3f( 20.0, 150.0, 25.0 );
		l.intensity = vec3f( 0.0, 0.0, 1.0 );
		rtScene.lights.push_back( l );
		*/
	}

	const size_t modelCnt = rtScene.models.size();
	for ( size_t m = 0; m < modelCnt; ++m )
	{
		RtModel& model = rtScene.models[ m ];
		rtScene.aabb.Expand( model.octree.GetAABB().min );
		rtScene.aabb.Expand( model.octree.GetAABB().max );
	}
}


void DrawGradientImage( ImageBuffer<Color>& image, const Color& color0, const Color& color1, const float power = 1.0f )
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
void WriteImage( const ImageBuffer<T>& image, const std::string& path, const int32_t number = -1 )
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


int raytracemain(void)
{
	std::cout << "Running Raytracer/Rasterizer" << std::endl;

	Timer loadTimer;
	Scene scene;
	RtScene rtScene;

	rtScene.scene = &scene;

	CreateMaterials( *rtScene.assets );

	loadTimer.Start();
	BuildRtSceneView( *rtScene.assets, rtScene );
	loadTimer.Stop();

	std::cout << "Load Time: " << loadTimer.GetElapsed() << "ms" << std::endl;

	dbg.diffuse = ImageBuffer<Color>( RenderWidth, RenderHeight, Color::Red, "dbgDiffuse" );
	dbg.normal = ImageBuffer<Color>( RenderWidth, RenderHeight, Color::White, "dbgNormal" );
	dbg.wireframe = ImageBuffer<Color>( RenderWidth, RenderHeight, Color::LGrey, "dbgWireframe" );
	dbg.topWire = ImageBuffer<Color>( RenderWidth, RenderHeight, Color::LGrey, "dbgTopWire" );
	dbg.sideWire = ImageBuffer<Color>( RenderWidth, RenderHeight, Color::LGrey, "dbgSideWire" );

	colorBuffer = ImageBuffer<Color>( RenderWidth, RenderHeight, Color::Black, "colorBuffer" );
	depthBuffer = ImageBuffer<float>( RenderWidth, RenderHeight, 0.0f, "depthBuffer" );

	ImageBuffer<Color> frameBuffer = ImageBuffer<Color>( RenderWidth, RenderHeight, Color::DGrey, "_frameBuffer" );
	DrawGradientImage( frameBuffer, Color::Blue, Color::Red, 0.8f );

	SetupViews();

	const int32_t imageCnt = 1;
	for ( int32_t i = 0; i < imageCnt; ++i )
	{
		Timer traceTimer;

		traceTimer.Start();
		TraceScene( rtViews[ VIEW_CAMERA ], rtScene, frameBuffer );
		traceTimer.Stop();

		RasterizeViews( rtScene );

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