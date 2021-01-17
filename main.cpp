
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
#include "../GfxCore/bitmap.h"
#include "../GfxCore/color.h"
#include "../GfxCore/mathVector.h"
#include "../GfxCore/matrix.h"
#include "../GfxCore/meshIO.h"
#include "../GfxCore/geom.h"
#include "../GfxCore/resourceManager.h"
#include "../GfxCore/camera.h"
#include "../GfxCore/image.h"
#include "../GfxCore/rasterLib.h"
#include "../GfxCore/util.h"
#include "scene.h"
#include "debug.h"
#include "globals.h"
#include "timer.h"

ResourceManager	rm;

material_t mirrorMaterial;
material_t diffuseMaterial;
material_t colorMaterial;

Scene			scene;
SceneView		views[4];
debug_t			dbg;
Image<Color>	colorBuffer;
Image<float>	depthBuffer;

extern Image<float> zBuffer;

void RasterScene( Image<Color>& image, const SceneView& view, bool wireFrame = true );

void ImageToBitmap( const Image<Color>& image, Bitmap& bitmap );
void ImageToBitmap( const Image<float>& image, Bitmap& bitmap );
void BitmapToImage( const Bitmap& bitmap, Image<Color>& image );

static Color skyColor = Color::Blue;

sample_t RecordSkyInfo( const Ray& r, const double t )
{
	sample_t sample;

	const double skyDot = Dot( r.GetVector(), vec3d( 0.0, 0.0, 1.0 ) );
	const Color gradient = Lerp( Color( Color::White ), skyColor, Saturate( skyDot ) );

	sample.color = gradient;
	sample.albedo = gradient;
	sample.normal = vec3d( 0.0 );
	sample.hitCode = HIT_SKY;
	sample.modelIx = ResourceManager::InvalidModelIx;
	sample.pt = vec3d( 0.0 );
	sample.surfaceDot = 0.0;
	sample.t = t;

	return sample;
}


sample_t RecordSurfaceInfo( const Ray& r, const double t, const uint32_t triIndex, const uint32_t modelIx )
{
	const ModelInstance& model = scene.models[ modelIx ];
	const std::vector<Triangle>& triCache = model.triCache;
	const Triangle& tri = triCache[ triIndex ];

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

	sample.albedo = sample.color;
	if( model.material.textured )
	{
		const Image<Color>* texture = rm.GetImageRef( model.material.colorMapId );
		vec2d uv = b[ 0 ] * tri.v0.uv + b[ 1 ] * tri.v1.uv + b[ 2 ] * tri.v2.uv;
		sample.albedo = texture->GetPixel( uv[ 0 ] * texture->GetWidth(), uv[ 1 ] * texture->GetHeight() );
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
	outSample.t = DBL_MAX;
	outSample.hitCode = HIT_NONE;

	int hitCnt = 0;

	const uint32_t modelCnt = scene.models.size();
	for ( uint32_t modelIx = 0; modelIx < modelCnt; ++modelIx )
	{
		const ModelInstance& model = scene.models[ modelIx ];

#if USE_AABB
		double t0 = 0.0;
		double t1 = 0.0;
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

			double t;
			bool isBackface;
			if( RayToTriangleIntersection( ray, tri, isBackface, t ) )
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
	double tnear = 0;
	double tfar = 0;
	
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
	if( !IntersectScene( ray, true, false, surfaceSample ) )
	{
		sample = RecordSkyInfo( ray, surfaceSample.t );
		return sample;
	}
	else
	{
		Color finalColor = Color::Black;
		const material_t& material = scene.models[ surfaceSample.modelIx ].material;
		Color surfaceColor = material.textured ? surfaceSample.albedo : surfaceSample.color;		

		vec3d viewVector = ray.GetVector().Reverse();
		viewVector = viewVector.Normalize();

		Color relfectionColor = Color::Black;
#if USE_RELFECTION
		if ( ( rayDepth < MaxBounces ) && ( material.Tr > 0.0 ) )
		{
			const vec3d reflectVector = 2.0 * Dot( viewVector, surfaceSample.normal ) * surfaceSample.normal - viewVector;

			Ray reflectionRay = Ray( surfaceSample.pt, surfaceSample.pt + reflectVector );

			reflectionRay.maxt = DBL_MAX;//std::max( 0.0, reflectionVector.t - reflectionVector.mint );

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
			vec3d& lightPos = scene.lights[ li ].pos;

			
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
				const double intensity = scene.lights[ li ].intensity;

				vec3d lightDir = shadowRay.GetVector();
				lightDir = lightDir.Normalize();

				const vec3d halfVector = ( viewVector + lightDir ).Normalize();

				const double diffuseIntensity = material.Kd * intensity * std::max( 0.0, Dot( lightDir, surfaceSample.normal ) );

				const double specularIntensity = material.Ks * pow( std::max( 0.0, Dot( surfaceSample.normal, halfVector ) ), SpecularPower );

				shadingColor = ( (float) diffuseIntensity * surfaceColor ) + Color( (float) specularIntensity );
			}

			finalColor += shadingColor + relfectionColor;
		}

		const Color ambient = AmbientLight * ( (float) material.Ka * surfaceColor );

		sample = surfaceSample;
		sample.color = finalColor + ambient;
		return sample;
	}

	return sample;
}


SceneView SetupFrontView()
{
	SceneView view;

	view.targetSize = RenderSize;
	view.camera = Camera(	vec4d( -280.0, -30.0, 50.0, 0.0 ),
							vec4d( 0.0, -1.0, 0.0, 0.0 ),
							vec4d( 0.0, 0.0, -1.0, 0.0 ),
							vec4d( -1.0, 0.0, 0.0, 0.0 ),
							CameraFov,
							AspectRatio( view.targetSize ),
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

	view.targetSize = RenderSize;
	view.camera = Camera(	vec4d( 0.0, 0.0, 280.0, 0.0 ),
							vec4d( 0.0, -1.0, 0.0, 0.0 ),
							vec4d( -1.0, 0.0, 0.0, 0.0 ),
							vec4d( 0.0, 0.0, 1.0, 0.0 ),
							CameraFov,
							AspectRatio( view.targetSize ),
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

	view.targetSize = RenderSize;
	view.camera = Camera(	vec4d( 0.0, 280.0, 0.0, 0.0 ),
							vec4d( -1.0, 0.0, 0.0, 0.0 ),
							vec4d( 0.0, 0.0, -1.0, 0.0 ),
							vec4d( 0.0, 1.0, 0.0, 0.0 ),
							CameraFov,
							AspectRatio( view.targetSize ),
							CameraNearPlane,
							CameraFarPlane );

	view.viewTransform = view.camera.ToViewMatrix();
	view.projTransform = view.camera.ToPerspectiveProjMatrix();
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


void TracePixel( const SceneView& view, Image<Color>& image, const uint32_t px, const uint32_t py )
{
#if USE_SS4X
	static const uint32_t subSampleCnt = 4;
	static const vec2d subPixelOffsets[ subSampleCnt ] = { vec2d( 0.25, 0.25 ), vec2d( 0.75, 0.25 ), vec2d( 0.25, 0.75 ), vec2d( 0.75, 0.75 ) };
#else
	static const uint32_t subSampleCnt = 1;
	static const vec2d subPixelOffsets[ subSampleCnt ] = { vec2d( 0.5, 0.5 ) };
#endif

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
		vec2d uv = vec2d( pixelXY[ 0 ] / ( view.targetSize[ 0 ] - 1.0 ), pixelXY[ 1 ] / ( view.targetSize[ 1 ] - 1.0 ) );

		Ray ray = views[ VIEW_FRONT ].camera.GetViewRay( uv );

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
		int32_t imageY = static_cast<int32_t>( py );

		coverage /= subSampleCnt;
		diffuse /= subSampleCnt;
		normal = normal.Normalize();
		t /= subSampleCnt;

		Color src = Color( LinearToSrgb( ( 1.0f / subSampleCnt ) * pixelColor ) );
		src.rgba().a = (float)coverage;

		// normal = normal.Reverse();
		Color normColor = Vec4dToColor( vec4d( 0.5 * normal + vec3d( 0.5 ), 1.0 ) );
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

	for ( uint32_t py = y0; py < y1; ++py )
	{
		if( py >= image->GetHeight() )
			return;

		for ( uint32_t px = x0; px < x1; ++px )
		{
			if ( px >= image->GetWidth() )
				return;

			TracePixel( view, *image, px, py );
		}
	}
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
		std::cout << static_cast<int>( 100.0 * ( threadsComplete / (double)threadsLaunched ) ) << "% ";
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


void CreateMaterials()
{
	mirrorMaterial.Ka = 0.1;
	mirrorMaterial.Kd = 0.1;
	mirrorMaterial.Ks = 1.0;
	mirrorMaterial.Ke = 1.0;
	mirrorMaterial.Tr = 1.0f;

	diffuseMaterial.Ka = 1.0;
	diffuseMaterial.Kd = 1.0;
	diffuseMaterial.Ks = 1.0;
	diffuseMaterial.Ke = 1.0;
	diffuseMaterial.Tr = 0.0f;

	colorMaterial.Ka = 1.0;
	colorMaterial.Kd = 1.0;
	colorMaterial.Ks = 1.0;
	colorMaterial.Ke = 1.0;
	colorMaterial.Tr = 0.0f;
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
		mat4x4d modelMatrix;
		
		ModelInstance teapot0;
		modelMatrix = BuildModelMatrix( vec3d( 30.0, 120.0, 10.0 ), vec3d( 0.0, 0.0, -90.0 ), 1.0, RHS_XZY );
		CreateModelInstance( modelIx, modelMatrix, true, Color::Yellow, &teapot0, colorMaterial );
		scene.models.push_back( teapot0 );
		
		ModelInstance teapot1;
		modelMatrix = BuildModelMatrix( vec3d( -30.0, -50.0, 10.0 ), vec3d( 0.0, 0.0, 30.0 ), 1.0, RHS_XZY );
		CreateModelInstance( modelIx, modelMatrix, true, Color::Green, &teapot1, colorMaterial );
		scene.models.push_back( teapot1 );
	}
	*/

	rm.PushVB( vb );
	rm.PushIB( ib );

	modelIx = LoadModelObj( std::string( "models/sphere.obj" ), rm );
	if( modelIx >= 0 )
	{
		mat4x4d modelMatrix;

		ModelInstance sphere0;
		modelMatrix = BuildModelMatrix( vec3d( 30.0, 40.0, 0.0 ), vec3d( 0.0, 0.0, 0.0 ), 0.8, RHS_XZY );
		CreateModelInstance( rm, modelIx, modelMatrix, true, Color::White, &sphere0, &mirrorMaterial );
		scene.models.push_back( sphere0 );

		ModelInstance sphere1;
		modelMatrix = BuildModelMatrix( vec3d( -50.0, -10.0, 10.0 ), vec3d( 0.0, 0.0, 0.0 ), 0.3, RHS_XZY );
		CreateModelInstance( rm, modelIx, modelMatrix, true, Color::Red, &sphere1 );
		scene.models.push_back( sphere1 );
	}

	modelIx = LoadModelBin( std::string( "models/skull.mdl" ), rm );
	if ( modelIx >= 0 )
	{
		mat4x4d modelMatrix;

		ModelInstance skull0;
		modelMatrix = BuildModelMatrix( vec3d( 30.0, 120.0, 10.0 ), vec3d( 0.0, 90.0, 40.0 ), 4.0, RHS_XZY );
		CreateModelInstance( rm, modelIx, modelMatrix, true, Color::Gold, &skull0, &mirrorMaterial );
		scene.models.push_back( skull0 );

		ModelInstance skull1;
		modelMatrix = BuildModelMatrix( vec3d( -30.0, -120.0, -10.0 ), vec3d( 0.0, 90.0, 0.0 ), 5.0, RHS_XZY );
		CreateModelInstance( rm, modelIx, modelMatrix, true, Color::Gold, &skull1 );
		scene.models.push_back( skull1 );		
	}

	modelIx = CreatePlaneModel( rm, vec2d( 500.0 ), vec2i( 1 ) );
	if ( modelIx >= 0 )
	{
		mat4x4d modelMatrix;

		ModelInstance plane0;
		modelMatrix = BuildModelMatrix( vec3d( 0.0, 0.0, -10.0 ), vec3d( 0.0, 0.0, 0.0 ), 1.0, RHS_XYZ );
		CreateModelInstance( rm, modelIx, modelMatrix, false, Color::White, &plane0, &mirrorMaterial );
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

	CreateMaterials();

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