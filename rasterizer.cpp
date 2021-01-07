#include "bitmap.h"
#include "mathVector.h"
#include "matrix.h"
#include "scene.h"
#include "debug.h"
#include "globals.h"
#include "image.h"

static Image<float> zBuffer( RenderWidth, RenderHeight, 1.0f, "_zbuffer" );

//
#include <map>
static std::map<float,float> zBufferValuesDbg;
//

extern Scene scene;
extern Bitmap* depthBuffer;

void OrthoMatrixToAxis( const mat4x4d& m, vec3d& origin, vec3d& xAxis, vec3d& yAxis, vec3d& zAxis );
void DrawWorldAxis( Bitmap& bitmap, const SceneView& view, double size, const vec3d& origin, const vec3d& X, const vec3d& Y, const vec3d& Z );

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
		{ 6, 7 },
		// Sides
		{ 0, 4 },
		{ 1, 5 },
		{ 2, 6 },
		{ 3, 7 },
	};

	vec2i ssPts[ 8 ];

	for ( int i = 0; i < 8; ++i )
	{
		vec4d pt;
		ProjectPoint( view.projView, RenderSize, true, corners[ i ], pt );
		ssPts[ i ] = vec2i( static_cast<int32_t>( pt[ 0 ] ), static_cast<int32_t>( pt[ 1 ] ) );
	}

	for ( int i = 0; i < 12; ++i )
	{
		vec2i  pt0 = ssPts[ edges[ i ][ 0 ] ];
		vec2i  pt1 = ssPts[ edges[ i ][ 1 ] ];
		Color color = Color::Green;
		color.a() = 0.4f;
		DrawLine( bitmap, pt0[ 0 ], pt0[ 1 ], pt1[ 0 ], pt1[ 1 ], color );
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
		vec4d pt;
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
	vec4d projPt;
	ProjectPoint( view.projView, vec2i( RenderWidth, RenderHeight ), true, point, projPt );

	vec2i ssPt = vec2i( static_cast<int32_t>( projPt[ 0 ] ), static_cast<int32_t>( projPt[ 1 ] ) );
	vec2d halfPt = vec2d( 0.5 * ( (double)size + 0.5 ) );
	vec2d start = Trunc<4, 2>( projPt ) - halfPt;

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
	vec4d ssPt[ 2 ];
	vec4d wsPt[ 2 ];
	wsPt[ 0 ] = vec4d( ray.o, 1.0 );
	wsPt[ 1 ] = vec4d( ray.GetEndPoint(), 1.0 );
	ProjectPoint( view.projView, vec2i( RenderWidth, RenderHeight ), true, wsPt[ 0 ], ssPt[ 0 ] );
	ProjectPoint( view.projView, vec2i( RenderWidth, RenderHeight ), true, wsPt[ 1 ], ssPt[ 1 ] );
	DrawLine( bitmap, (int)ssPt[ 0 ][ 0 ], (int)ssPt[ 0 ][ 1 ], (int)ssPt[ 1 ][ 0 ], (int)ssPt[ 1 ][ 1 ], color.AsR8G8B8A8() );
}


void RasterScene( Bitmap& bitmap, const SceneView& view, bool wireFrame = true )
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
			vec4d ssPts[ 3 ];
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

#if USE_RASTERIZE
			if ( !wireFrame )
			{
				// Scanline Rasterizer
				AABB ssBox;
				for ( int i = 0; i < 3; ++i )
				{
					ssBox.Expand( Trunc<4, 1>( ssPts[ i ] ) );
				}

				const vec3d tPt0 = Trunc<4, 1>( ssPts[ 0 ] );
				const vec3d tPt1 = Trunc<4, 1>( ssPts[ 1 ] );
				const vec3d tPt2 = Trunc<4, 1>( ssPts[ 2 ] );

				const int32_t x0 = static_cast<int>( ssBox.min[ 0 ] );
				const int32_t x1 = static_cast<int>( ssBox.max[ 0 ] + 0.5 );
				const int32_t y0 = static_cast<int>( ssBox.min[ 1 ] );
				const int32_t y1 = static_cast<int>( ssBox.max[ 1 ] + 0.5 );

				for ( int32_t y = y0; y <= y1; ++y )
				{
					for ( int32_t x = x0; x <= x1; ++x )
					{
						const vec3d baryPt = PointToBarycentric( vec3d( x, y, 0.0 ), tPt0, tPt1, tPt2 );

						float depth = (float)Interpolate( baryPt, ssPts )[ 2 ];
						const vec3d wsPoint = vec3d( x, y, depth );

						const vec3d wsBaryPt = PointToBarycentric( vec3d( x, y, depth ), Trunc<4, 1>( triList[ i ].v0.pos ), Trunc<4, 1>( triList[ i ].v1.pos ), Trunc<4, 1>( triList[ i ].v2.pos ) );

						if ( ( baryPt[ 0 ] >= 0 ) && ( baryPt[ 1 ] >= 0 ) && ( baryPt[ 2 ] >= 0 ) )
						{
							
							vec3d normal = baryPt[ 0 ] * triList[ i ].v0.normal;
							normal += baryPt[ 1 ] * triList[ i ].v1.normal;
							normal += baryPt[ 2 ] * triList[ i ].v2.normal;
							normal = normal.Normalize();
							
							//vec3d normal = triList[ i ].v0.normal.Normalize();

							vec3d lightDir = scene.lights[ 0 ].pos - wsPoint;
							lightDir = lightDir.Normalize();
							float diffuse = Saturate( Dot( normal, lightDir ) );

							const Ray viewRay = view.camera.GetViewRay( vec2d( x / (double)RenderWidth, y / (double)RenderHeight ) );

							const Color viewDiffuse = Color( (float)Dot( viewRay.d.Normalize().Reverse(), normal ) );

							Color color = baryPt[ 0 ] * triList[ i ].v0.color;
							color += baryPt[ 1 ] * triList[ i ].v1.color;
							color += baryPt[ 2 ] * triList[ i ].v2.color;
							color *= diffuse;
							color += 0.05f;

							const Color normalColor = Color( 0.5f * normal[ 0 ] + 0.5f, 0.5f * normal[ 1 ] + 0.5f, 0.5f * normal[ 2 ] + 0.5f );
							//const Color normalColor = Color( normal[ 0 ], normal[ 1 ], normal[ 2 ] );

							if ( depth < zBuffer.GetPixel( x, y ) )
							{
								bitmap.SetPixel( x, y, LinearToSrgb( color ).AsR8G8B8A8() );
								zBuffer.SetPixel( x, y, depth );
							}
						}
					}
				}
			}
			else
#endif
			{
				Color color = triList[ i ].v0.color;
				color.rgba().a = 0.1f;

				DrawLine( bitmap, pxPts[ 0 ][ 0 ], pxPts[ 0 ][ 1 ], pxPts[ 1 ][ 0 ], pxPts[ 1 ][ 1 ], color );
				DrawLine( bitmap, pxPts[ 0 ][ 0 ], pxPts[ 0 ][ 1 ], pxPts[ 2 ][ 0 ], pxPts[ 2 ][ 1 ], color );
				DrawLine( bitmap, pxPts[ 1 ][ 0 ], pxPts[ 1 ][ 1 ], pxPts[ 2 ][ 0 ], pxPts[ 2 ][ 1 ], color );
			}
		}
	}
#endif

	if ( wireFrame )
	{
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

	float minZ = FLT_MAX;
	float maxZ = -FLT_MAX;
	for ( int32_t y = 0; y < RenderHeight; ++y )
	{
		for ( int32_t x = 0; x < RenderWidth; ++x )
		{
			const float zValue = zBuffer.GetPixel( x, y );

			minZ = std::min( minZ, zValue );
			maxZ = std::max( maxZ, zValue );
		}
	}

	for ( int32_t y = 0; y < RenderHeight; ++y )
	{
		for ( int32_t x = 0; x < RenderWidth; ++x )
		{
			const float zValue = zBuffer.GetPixel( x, y );
			const float packedZ = ( zValue - minZ ) / ( maxZ - minZ );
			zBufferValuesDbg[ zValue ] = packedZ;

			const Color c = Color( packedZ );
			depthBuffer->SetPixel( x, y, c.AsR8G8B8A8() );
		}
	}
}