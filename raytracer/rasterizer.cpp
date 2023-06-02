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

#include <image/bitmap.h>
#include <math/vector.h>
#include <math/matrix.h>
#include <image/image.h>
#include <scene/resourceManager.h>
#include <acceleration/octree.h>
#include <core/util.h>
#include <core/rasterLib.h>
#include "scene.h"
#include "debug.h"
#include "globals.h"

CpuImage<float> zBuffer( RenderWidth, RenderHeight, 0.0f, "_zbuffer" );

extern CpuImage<float> depthBuffer;
extern ResourceManager rm;

static Material DefaultMaterial;

void OrthoMatrixToAxis( const mat4x4f& m, vec3f& origin, vec3f& xAxis, vec3f& yAxis, vec3f& zAxis );
void DrawWorldAxis( CpuImage<Color>& image, const RtView& view, float size, const vec3f& origin, const vec3f& X, const vec3f& Y, const vec3f& Z );

struct vertexOut_t
{
	vec4f	wsPosition[ 3 ];
	vec4f	viewPosition[ 3 ];
	vec4f	clipPosition[ 3 ];
	vec4f	ndc[ 3 ];
	vec3f	normal[ 3 ];
	vec2f	uv[ 3 ];
	Color	color[ 3 ];
};


struct fragmentInput_t
{
	vec4f	wsPosition;
	vec4f	clipPosition;
	vec3f	normal;
	vec2f	uv;
	Color	color;
};


void DrawCube( CpuImage<Color>& image, const RtView& view, const vec4f& minCorner, const vec4f& maxCorner, Color color = Color::Green )
{
	vec4f corners[ 8 ] = {
		// Bottom
		vec4f( minCorner[ 0 ], minCorner[ 1 ], minCorner[ 2 ], 1.0 ),
		vec4f( maxCorner[ 0 ], minCorner[ 1 ], minCorner[ 2 ], 1.0 ),
		vec4f( maxCorner[ 0 ], maxCorner[ 1 ], minCorner[ 2 ], 1.0 ),
		vec4f( minCorner[ 0 ], maxCorner[ 1 ], minCorner[ 2 ], 1.0 ),
		// Top
		vec4f( minCorner[ 0 ], minCorner[ 1 ], maxCorner[ 2 ], 1.0 ),
		vec4f( maxCorner[ 0 ], minCorner[ 1 ], maxCorner[ 2 ], 1.0 ),
		vec4f( maxCorner[ 0 ], maxCorner[ 1 ], maxCorner[ 2 ], 1.0 ),
		vec4f( minCorner[ 0 ], maxCorner[ 1 ], maxCorner[ 2 ], 1.0 ),
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
		vec4f pt;
		ProjectPoint( view.projView, view.targetSize, corners[ i ], pt );
		ssPts[ i ] = vec2i( static_cast<int32_t>( pt[ 0 ] ), static_cast<int32_t>( pt[ 1 ] ) );
	}

	for ( int i = 0; i < 12; ++i )
	{
		vec2i  pt0 = ssPts[ edges[ i ][ 0 ] ];
		vec2i  pt1 = ssPts[ edges[ i ][ 1 ] ];

		color.a() = 0.4f;
		DrawLine( image, pt0[ 0 ], pt0[ 1 ], pt1[ 0 ], pt1[ 1 ], color );
	}

	for ( int i = 0; i < 8; ++i )
	{
		image.SetPixel( ssPts[ i ][ 0 ], ssPts[ i ][ 1 ], Color::Red );
	}
}


void DrawWorldAxis( CpuImage<Color>& image, const RtView& view, float size, const vec3f& origin, const vec3f& X, const vec3f& Y, const vec3f& Z )
{
	vec4f points[ 4 ] = {
		{ vec4f( origin, 1.0 ) },
		{ vec4f( origin + size * X.Normalize(), 1.0 ) },
		{ vec4f( origin + size * Y.Normalize(), 1.0 ) },
		{ vec4f( origin + size * Z.Normalize(), 1.0 ) },
	};

	vec2i ssPts[ 4 ];
	for ( int i = 0; i < 4; ++i )
	{
		vec4f pt;
		ProjectPoint( view.projView, view.targetSize, points[ i ], pt );
		ssPts[ i ] = vec2i( static_cast<int32_t>( pt[ 0 ] ), static_cast<int32_t>( pt[ 1 ] ) );
	}

	DrawLine( image, ssPts[ 0 ][ 0 ], ssPts[ 0 ][ 1 ], ssPts[ 1 ][ 0 ], ssPts[ 1 ][ 1 ], Color::Red );
	DrawLine( image, ssPts[ 0 ][ 0 ], ssPts[ 0 ][ 1 ], ssPts[ 2 ][ 0 ], ssPts[ 2 ][ 1 ], Color::Green );
	DrawLine( image, ssPts[ 0 ][ 0 ], ssPts[ 0 ][ 1 ], ssPts[ 3 ][ 0 ], ssPts[ 3 ][ 1 ], Color::Blue );
	image.SetPixel( ssPts[ 0 ][ 0 ], ssPts[ 0 ][ 1 ], Color::Black );
}


void DrawWorldPoint( CpuImage<Color>& image, const RtView& view, const vec4f& point, const int32_t size, const Color& color )
{
	vec4f projPt;
	ProjectPoint( view.projView, view.targetSize, point, projPt );

	vec2i ssPt = vec2i( static_cast<int32_t>( projPt[ 0 ] ), static_cast<int32_t>( projPt[ 1 ] ) );
	vec2f halfPt = vec2f( 0.5f * ( (float)size + 0.5f ) );
	vec2f start = Trunc<4, 2>( projPt ) - halfPt;

	int32_t i0 = static_cast<int32_t>( start[ 0 ] );
	int32_t iN = i0 + size;

	int32_t j0 = static_cast<int32_t>( start[ 1 ] );
	int32_t jN = j0 + size;

	for ( int32_t j = j0; j < jN; ++j )
	{
		for ( int32_t i = i0; i < iN; ++i )
		{
			image.SetPixel( i, j, color );
		}
	}
}


void DrawRay( CpuImage<Color>& image, const RtView& view, const Ray& ray, const Color& color )
{
	vec4f ssPt[ 2 ];
	vec4f wsPt[ 2 ];
	wsPt[ 0 ] = vec4f( ray.o, 1.0 );
	wsPt[ 1 ] = vec4f( ray.GetEndPoint(), 1.0 );
	ProjectPoint( view.projView, view.targetSize, wsPt[ 0 ], ssPt[ 0 ] );
	ProjectPoint( view.projView, view.targetSize, wsPt[ 1 ], ssPt[ 1 ] );
	DrawLine( image, (int)ssPt[ 0 ][ 0 ], (int)ssPt[ 0 ][ 1 ], (int)ssPt[ 1 ][ 0 ], (int)ssPt[ 1 ][ 1 ], color.AsR8G8B8A8() );
}


template<typename T>
void DrawOctree( CpuImage<Color>& image, const RtView& view, const Octree<T>& octree, const Color& color )
{
	AABB bounds = octree.GetAABB();
	DrawCube( image, view, vec4f( bounds.min, 1.0 ), vec4f( bounds.max, 1.0 ), color );

	auto nodeEnd = octree.children.end();
	for ( auto node = octree.children.begin(); node != nodeEnd; ++node )
	{
		DrawOctree( image, view, *node, color );
	}
}


bool VertexShader( const RtView& view, const Triangle& tri, vertexOut_t& outVertex )
{
	const mat4x4f& mvp = view.projView;

	const vec4f* wsPts[ 3 ];
	vec4f ssPts[ 3 ];
	bool culled = false;

	wsPts[ 0 ] = &tri.v0.pos;
	wsPts[ 1 ] = &tri.v1.pos;
	wsPts[ 2 ] = &tri.v2.pos;

	ProjectPoint( mvp, view.targetSize, *wsPts[ 0 ], ssPts[ 0 ] );
	ProjectPoint( mvp, view.targetSize, *wsPts[ 1 ], ssPts[ 1 ] );
	ProjectPoint( mvp, view.targetSize, *wsPts[ 2 ], ssPts[ 2 ] );

	const bool nearClip = ( ssPts[ 0 ][ 2 ] < -1.0 ) && ( ssPts[ 1 ][ 2 ] < -1.0 ) && ( ssPts[ 2 ][ 2 ] < -1.0 );

	if( !culled && !nearClip )
	{
		outVertex.clipPosition[ 0 ] = ssPts[ 0 ];
		outVertex.wsPosition[ 0 ] = tri.v0.pos;
		outVertex.color[ 0 ] = tri.v0.color;
		outVertex.uv[ 0 ] = tri.v0.uv;
		outVertex.normal[ 0 ] = tri.v0.normal;

		outVertex.clipPosition[ 1 ] = ssPts[ 1 ];
		outVertex.wsPosition[ 1 ] = tri.v1.pos;
		outVertex.color[ 1 ] = tri.v1.color;
		outVertex.uv[ 1 ] = tri.v1.uv;
		outVertex.normal[ 1 ] = tri.v1.normal;

		outVertex.clipPosition[ 2 ] = ssPts[ 2 ];
		outVertex.wsPosition[ 2 ] = tri.v2.pos;
		outVertex.color[ 2 ] = tri.v2.color;
		outVertex.uv[ 2 ] = tri.v2.uv;
		outVertex.normal[ 2 ] = tri.v2.normal;

		return true;
	}
	else
	{
		return false;
	}
}


bool EmitFragment( const vec3f& baryPt, const vertexOut_t& vo, fragmentInput_t& outFragment )
{
	if( ( baryPt[ 0 ] < 0.0 ) || ( baryPt[ 1 ] < 0.0 ) || ( baryPt[ 2 ] < 0.0 ) )
	{
		return false;
	}

	if ( ( baryPt[ 0 ] > 1.0 ) || ( baryPt[ 1 ] > 1.0 ) || ( baryPt[ 2 ] > 1.0 ) )
	{
	//	return false;
	}

	outFragment.wsPosition = Interpolate( baryPt, vo.wsPosition ); // TODO: is this ok?
	outFragment.clipPosition = Interpolate( baryPt, vo.clipPosition );
	outFragment.normal = Interpolate( baryPt, vo.normal );
	outFragment.uv = Interpolate( baryPt, vo.uv );
	outFragment.color = Interpolate( baryPt, vo.color );

	return true;
}


bool PixelShader( const fragmentInput_t & frag )
{
	return false;
}


void RasterScene( CpuImage<Color>& image, const RtView& view, const RtScene& rtScene, bool wireFrame = true )
{
	mat4x4f mvp = view.projTransform * view.viewTransform;
	const uint32_t modelCnt = static_cast<uint32_t>( rtScene.models.size() );

#if DRAW_WIREFRAME
	for ( uint32_t m = 0; m < modelCnt; ++m )
	{
		const RtModel& model = rtScene.models[ m ];
		const Triangle* triCache = model.triCache.data();

		const size_t triCnt = model.triCache.size();
		for ( uint32_t i = 0; i < triCnt; ++i )
		{
			vertexOut_t vo;
			if ( !VertexShader( view, triCache[ i ], vo ) )
			{
				continue;
			}

#if USE_RASTERIZE
			if ( !wireFrame )
			{
				// Scanline Rasterizer
				AABB ssBox;
				for ( int i = 0; i < 3; ++i )
				{
					ssBox.Expand( Trunc<4, 1>( vo.clipPosition[ i ] ) );
				}

				const vec3f tPt0 = Trunc<4, 1>( vo.clipPosition[ 0 ] );
				const vec3f tPt1 = Trunc<4, 1>( vo.clipPosition[ 1 ] );
				const vec3f tPt2 = Trunc<4, 1>( vo.clipPosition[ 2 ] );

				const int32_t x0 = std::max( 0,										static_cast<int>( ssBox.min[ 0 ] ) );
				const int32_t x1 = std::min( static_cast<int>( image.GetWidth() ),	static_cast<int>( ssBox.max[ 0 ] + 0.5 ) );
				const int32_t y0 = std::max( 0,										static_cast<int>( ssBox.min[ 1 ] ) );
				const int32_t y1 = std::min( static_cast<int>( image.GetHeight() ),	static_cast<int>( ssBox.max[ 1 ] + 0.5 ) );

				for ( int32_t y = y0; y <= y1; ++y )
				{
					for ( int32_t x = x0; x <= x1; ++x )
					{
						const vec3f baryPt = PointToBarycentric( vec3f( (float)x, (float)y, 0.0 ), tPt0, tPt1, tPt2 );

						fragmentInput_t fragmentInput;
						if( !EmitFragment( baryPt, vo, fragmentInput ) )
							continue;

						const float depth = (float)fragmentInput.clipPosition[ 2 ];

						if ( depth < zBuffer.GetPixel( x, y ) )
							continue;

						const vec3f normal = fragmentInput.normal.Normalize();

						const light_t& L = rtScene.lights[ 0 ];
						const vec4f intensity = L.intensity;

						vec3f lightDir = Trunc<4, 1>( L.lightPos - fragmentInput.wsPosition );
						lightDir = lightDir.Normalize();

						const vec3f viewVector = Trunc<4, 1>( view.camera.GetOrigin() - fragmentInput.wsPosition ).Normalize();
						const Color viewDiffuse = Color( (float)Dot( viewVector, normal ) );

						const vec3f halfVector = ( viewVector + lightDir ).Normalize();

						Color surfaceColor = Color::Black;

						const Material& material = rtScene.assets->materialLib.Find( triCache[ i ].materialId )->Get();

						if( material.IsTextured() )
						{
							const Image& texture = rtScene.assets->textureLib.Find( material.GetTexture( GGX_COLOR_MAP_SLOT ) )->Get();
						//	surfaceColor = texture.cpuImage.GetPixelUV( fragmentInput.uv[ 0 ], fragmentInput.uv[ 1 ] ); // FIXME
						}
						else
						{
							surfaceColor += fragmentInput.color;
						}

						const vec4f D = ColorToVector( Color( material.Kd() ) );
						const vec4f S = ColorToVector( Color( material.Ks() ) );

						const vec4f diffuseIntensity = Multiply( D, intensity ) * std::max( 0.0f, Dot( normal, lightDir ) );
						const vec4f specularIntensity = S * pow( std::max( 0.0f, Dot( normal, halfVector ) ), material.Ns() );
						const Color ambient = AmbientLight * ( Color( material.Ka() ) * surfaceColor );

						Color shadingColor;
						shadingColor = Vec4ToColor( specularIntensity );
						shadingColor += Vec4ToColor( Multiply( diffuseIntensity, ColorToVector( surfaceColor ) ) );
						shadingColor += ambient;

						shadingColor = Vec3ToColor( BrdfGGX( normal, viewVector, lightDir, material ) );

						const Color normalColor = Vec3ToColor( 0.5f * normal + vec3f( 0.5f ) );
						
						image.SetPixel( x, y, LinearToSrgb( shadingColor ).AsR8G8B8A8() );
						zBuffer.SetPixel( x, y, depth );
					}
				}
			}
			else
#endif
			{
				Color color = vo.color[ 0 ];
				color.rgba().a = 0.1f;

				vec2i pxPts[ 3 ];
				pxPts[ 0 ] = vec2i( static_cast<int32_t>( vo.clipPosition[ 0 ][ 0 ] ), static_cast<int32_t>( vo.clipPosition[ 0 ][ 1 ] ) );
				pxPts[ 1 ] = vec2i( static_cast<int32_t>( vo.clipPosition[ 1 ][ 0 ] ), static_cast<int32_t>( vo.clipPosition[ 1 ][ 1 ] ) );
				pxPts[ 2 ] = vec2i( static_cast<int32_t>( vo.clipPosition[ 2 ][ 0 ] ), static_cast<int32_t>( vo.clipPosition[ 2 ][ 1 ] ) );

				pxPts[0][0] = Clamp( pxPts[0][0], 0, (int)image.GetWidth() );
				pxPts[0][1] = Clamp( pxPts[0][1], 0, (int)image.GetHeight() );
				pxPts[1][0] = Clamp( pxPts[1][0], 0, (int)image.GetWidth() );
				pxPts[1][1] = Clamp( pxPts[1][1], 0, (int)image.GetHeight() );
				pxPts[2][0] = Clamp( pxPts[1][0], 0, (int)image.GetWidth() );
				pxPts[2][1] = Clamp( pxPts[1][1], 0, (int)image.GetHeight() );

				DrawLine( image, pxPts[ 0 ][ 0 ], pxPts[ 0 ][ 1 ], pxPts[ 1 ][ 0 ], pxPts[ 1 ][ 1 ], color );
				DrawLine( image, pxPts[ 0 ][ 0 ], pxPts[ 0 ][ 1 ], pxPts[ 2 ][ 0 ], pxPts[ 2 ][ 1 ], color );
				DrawLine( image, pxPts[ 1 ][ 0 ], pxPts[ 1 ][ 1 ], pxPts[ 2 ][ 0 ], pxPts[ 2 ][ 1 ], color );
			}
		}
	}
#endif

	if ( wireFrame )
	{
		for ( uint32_t m = 0; m < modelCnt; ++m )
		{
			const RtModel& model = rtScene.models[ m ];
#if DRAW_AABB
			const AABB bounds = model.octree.GetAABB();
			DrawCube( image, view, vec4f( bounds.min, 1.0 ), vec4f( bounds.max, 1.0 ) );
			DrawCube( image, view, vec4f( bounds.min, 1.0 ), vec4f( bounds.max, 1.0 ) );
#endif
			vec3f origin;
			vec3f xAxis;
			vec3f yAxis;
			vec3f zAxis;
			OrthoMatrixToAxis( model.transform, origin, xAxis, yAxis, zAxis );
			DrawWorldAxis( image, view, 20.0f, origin, xAxis, yAxis, zAxis );
		}
	}
	
	// DrawOctree( image, view, rtScene.models[ 0 ].octree, Color::Red );
}