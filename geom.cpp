#include <map>
#include <deque>
#include <string>
#include <sstream>

#include "geom.h"
#include "resourceManager.h"
#include "..\GfxCore\bitmap.h"
#include "image.h"

void BitmapToImage( const Bitmap& bitmap, Image<Color>& image );

extern ResourceManager rm;

uint32_t LoadModel( const std::string& path, const uint32_t vb, const uint32_t ib )
{
	MeshIO::Off offMesh;
	MeshIO::ReadOFF( path, offMesh );

	uint32_t modelIx = rm.AllocModel();
	Model* model = rm.GetModel( modelIx );

	model->name = path;
	model->vb = vb;
	model->ib = ib;
	model->vbOffset = rm.GetVbOffset( vb );
	model->ibOffset = rm.GetIbOffset( ib );

	for ( int32_t i = 0; i < offMesh.verticesCnt; ++i )
	{
		vertex_t v;
		v.pos = vec4d( offMesh.vertices[ i ].pos.x, offMesh.vertices[ i ].pos.y, offMesh.vertices[ i ].pos.z, 1.0 );

		Color c( (float)offMesh.vertices[ i ].r, (float)offMesh.vertices[ i ].b, (float)offMesh.vertices[ i ].g, (float)offMesh.vertices[ i ].a );
		v.color = c.AsR8G8B8A8();

		rm.AddVertex( vb, v );
	}
	model->vbEnd = rm.GetVbOffset( vb );

	const size_t triCnt = offMesh.facesCnt;
	for ( size_t i = 0; i < triCnt; ++i )
	{
		const MeshIO::Polytope& face = offMesh.faces[ i ];

		rm.AddIndex( ib, model->vbOffset + face.points[ 0 ] );
		rm.AddIndex( ib, model->vbOffset + face.points[ 1 ] );
		rm.AddIndex( ib, model->vbOffset + face.points[ 2 ] );
	}
	model->ibEnd = rm.GetIbOffset( ib );

	model->material.Ka = 1.0;
	model->material.Tf = 0.5;
	model->material.Kd = 0.5;
	model->material.Ks = 1.0;
	model->material.Tr = 0.4;

	return modelIx;
}


uint32_t LoadModelObj( const std::string& path, const uint32_t vb, const uint32_t ib )
{
	/////////////////////////////////////////////
	//                                         //
	// Load model and clean data			   //
	//                                         //
	/////////////////////////////////////////////

	MeshIO::Obj objMesh;
	MeshIO::ReadObj( path, objMesh );

	auto& groups = objMesh.groups;

	// Format obj into vertices and indices
	std::vector<vertex_t> uniqueVertices; // TODO: replace with set or map
	std::vector<int32_t> indices;

	for ( auto group = groups.begin(); group != groups.end(); ++group )
	{
		auto& smoothingGroup = group->second.smoothingGroups;
		for ( auto smoothGroup = smoothingGroup.begin(); smoothGroup != smoothingGroup.end(); ++smoothGroup )
		{
			std::vector<MeshIO::objFace_t>& faces = smoothGroup->second.faces;

			faces.reserve( 10000 );

			const int32_t faceCnt = faces.size();
			for ( int32_t faceIx = 0; faceIx < faceCnt; ++faceIx )
			{
				const int32_t vertexCnt = faces[ faceIx ].vertices.size();
				assert( ( vertexCnt == 3 ) || ( vertexCnt == 4 ) );
				for ( int32_t i = 0; i < vertexCnt; ++i )
				{
					vertex_t vert;
					const int32_t vertIx = faces[ faceIx ].vertices[ i ].vertexIx;
					const int32_t uvIx = faces[ faceIx ].vertices[ i ].uvIx;
					const int32_t normalIx = faces[ faceIx ].vertices[ i ].normalIx;

					MeshIO::vector_t srcVertex = objMesh.vertices[ vertIx ];
					
					vert.pos = vec4d( srcVertex.x, srcVertex.y, srcVertex.z, srcVertex.w );
					vert.color = Color::White;

					if( uvIx != -1 )
					{
						MeshIO::vector_t srcUv = objMesh.uvs[ uvIx ];
						vert.uv = vec2d( srcUv.x, srcUv.y );
					}
					else
					{
						vert.uv = vec2d( 0.0, 0.0 );
					}

					if ( normalIx != -1 )
					{
						MeshIO::vector_t srcNormal = objMesh.normals[ normalIx ];
						vert.normal = vec3d( srcNormal.x, srcNormal.y, srcNormal.z ).Normalize();
					}
					else
					{
						vert.normal = vec3d( 1.0, 1.0, 1.0 ).Normalize();
					}

					auto it = std::find( uniqueVertices.begin(), uniqueVertices.end(), vert );

					if ( it == uniqueVertices.end() )
					{
						indices.push_back( uniqueVertices.size() );
						uniqueVertices.push_back( vert );
					}
					else
					{
						indices.push_back( std::distance( uniqueVertices.begin(), it ) );
					}
				}

				if( vertexCnt == 4 )
				{					
					// For quads do a very simple triangulation by appending the two
					// adjacent indices to make the second triangle in the quad.
					// This totals 6 indices, 3 for each triangle
					// TODO: winding order
					assert( indices.size() >= 4 );
					const uint32_t v0 = *( indices.end() - 4 );
					const uint32_t v2 = *( indices.end() - 2 );

					indices.push_back( v0 );
					indices.push_back( v2 );
				}
			}
		}
	}

	// Normalize UVs to [0, 1]
	// TODO: leave as-is and let texture wrap mode deal with it?
	{
		auto vertEnd = uniqueVertices.end();
		for( auto it = uniqueVertices.begin(); it != vertEnd; ++it )
		{
			it->uv[ 0 ] = ( it->uv[ 0 ] > 1.0 ) ? ( it->uv[ 0 ] - floor( it->uv[ 0 ] ) ) : it->uv[ 0 ];
			it->uv[ 1 ] = ( it->uv[ 1 ] > 1.0 ) ? ( it->uv[ 1 ] - floor( it->uv[ 1 ] ) ) : it->uv[ 1 ];

			it->uv[ 0 ] = Saturate( it->uv[ 0 ] );
			it->uv[ 1 ] = Saturate( it->uv[ 1 ] );

			it->uv[ 1 ] = 1.0 - it->uv[ 1 ];
		}
	}

	/////////////////////////////////////////////
	//                                         //
	// Construct final object representation   //
	//                                         //
	/////////////////////////////////////////////

	const uint32_t modelIx = rm.AllocModel();
	Model* model = rm.GetModel( modelIx );

	// Build VB and IB
	{
		model->name = path;
		model->vb = vb;
		model->ib = ib;
		model->vbOffset = rm.GetVbOffset( vb );
		model->ibOffset = rm.GetIbOffset( ib );

		const uint32_t vertexCnt = uniqueVertices.size();
		for ( int32_t i = 0; i < vertexCnt; ++i )
		{
			rm.AddVertex( vb, uniqueVertices[ i ] );
		}
		model->vbEnd = rm.GetVbOffset( vb );

		const size_t indexCnt = indices.size();
		assert( ( indexCnt % 3 ) == 0 );
		for ( size_t i = 0; i < indexCnt; i++ )
		{
			rm.AddIndex( ib, model->vbOffset + indices[ i ] );
		}
		model->ibEnd = rm.GetIbOffset( ib );
	}

	// Set material
	assert( objMesh.materialLibs.size() <= 1 ); // Just assume 1 material for now
	if( objMesh.materialLibs.size() > 0 )
	{
		std::string& matLib = objMesh.materialLibs[ 0 ];

		MeshIO::objMaterial_t material;
		MeshIO::ReadMtl( std::string( "models/" + matLib ), material );

		model->material.Ni = material.Ni;
		model->material.Ns = material.Ns;
		model->material.Ka = material.Ka.x;	
		model->material.Ke = material.Ke.x;
		model->material.Kd = material.Kd.x;
		model->material.Ks = material.Ks.x;
		model->material.Tf = material.Tf.x;
		model->material.Tr = material.Tr;
		model->material.illum = material.illum;
		model->material.textured = false;

		if( material.map_Kd.size() > 0 )
		{
			Bitmap texture = Bitmap( std::string( "textures/" + material.map_Kd ) );

			Image<Color> image = Image<Color>( texture.GetWidth(), texture.GetHeight(), Color( 0.0f ), material.map_Kd.c_str() );
			BitmapToImage( texture, image );
			model->material.colorMapId = rm.StoreImageCopy( image );
			model->material.textured = true;
		}
	}

	// MeshIO::WriteObj( std::string( "models/teapot-outtest.obj" ), objMesh );
	return modelIx;
}


void StoreModelObj( const std::string& path, const uint32_t modelIx )
{
	const Model* model = rm.GetModel( modelIx );

	MeshIO::Obj meshObj;
	for ( uint32_t i = model->vbOffset; i < model->vbEnd; ++i )
	{
		const vertex_t* v = rm.GetVertex( model->vb, i );

		MeshIO::vector_t vert;
		vert.x = v->pos[ 0 ];
		vert.y = v->pos[ 1 ];
		vert.z = v->pos[ 2 ];
		vert.w = v->pos[ 3 ];
		meshObj.vertices.push_back( vert );

		MeshIO::vector_t normal;
		normal.x = v->normal[ 0 ];
		normal.y = v->normal[ 1 ];
		normal.z = v->normal[ 2 ];
		normal.w = 0.0;
		meshObj.normals.push_back( normal );

		MeshIO::vector_t uv;
		uv.x = v->uv[ 0 ];
		uv.y = v->uv[ 1 ];
		uv.z = 0.0;
		uv.w = 0.0;
		meshObj.uvs.push_back( uv );
	}

	MeshIO::objGroup_t group;
	MeshIO::objSmoothingGroup_t smoothingGroup;

	group.material = "default";

	for ( uint32_t i = model->ibOffset; i < model->ibEnd; i += 3 )
	{
		MeshIO::objFace_t face;
		for( uint32_t j = 0; j < 3; ++ j )
		{
			const uint32_t index = rm.GetIndex( model->vb, i + j ) - model->vbOffset;

			MeshIO::objIndex_t indexTuple;
			indexTuple.vertexIx = index;
			indexTuple.uvIx = index;
			indexTuple.normalIx = index;
		
			// TODO: deduplicate
			face.vertices.push_back( indexTuple );
		}

		smoothingGroup.faces.push_back( face );
	}

	group.smoothingGroups[ 0 ] = smoothingGroup;
	meshObj.groups[ model->name ] = group;

	MeshIO::WriteObj( path, meshObj );
}


void CreateModelInstance( const uint32_t modelIx, const mat4x4d& modelMatrix, const bool smoothNormals, const Color& tint, ModelInstance* outInstance, const objMaterial_t* material )
{
	const Model* model = rm.GetModel( modelIx );

	outInstance->transform = modelMatrix;
	const uint32_t vb = rm.AllocVB();
	outInstance->modelIx = modelIx;
	outInstance->triCache.reserve( ( model->ibEnd - model->ibOffset ) / 3 );

	const uint32_t vbOffset = 0;

	using triIndices = std::tuple<uint32_t, uint32_t, uint32_t>;
	std::map< uint32_t, std::deque<triIndices> > vertToPolyMap;

	vec3d centroid = vec3d( 0.0, 0.0, 0.0 );
	for ( uint32_t i = model->vbOffset; i < model->vbEnd; ++i )
	{
		vertex_t vertex = *rm.GetVertex( model->vb, i );

		vertex.pos = outInstance->transform * vec4d( Trunc<4,1>( vertex.pos ), 1.0 );
		vertex.color *= tint;
		rm.AddVertex( vb, vertex );

		centroid += Trunc<4, 1>( vertex.pos );
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
			vec3d interpretedTangent = vec3d( 0.0, 0.0, 0.0 );
			vec3d interpretedBitangent = vec3d( 0.0, 0.0, 0.0 );

			vertex_t* vertex = rm.GetVertex( vb, iter->first );

			for ( std::deque<triIndices>::iterator polyListIter = iter->second.begin(); polyListIter != iter->second.end(); ++polyListIter )
			{
				const uint32_t i0 = std::get<0>( *polyListIter );
				const uint32_t i1 = std::get<1>( *polyListIter );
				const uint32_t i2 = std::get<2>( *polyListIter );

				// These are transformed positions; this is critical for proper normals
				const vec3d pt0 = Trunc<4, 1>( rm.GetVertex( vb, i0 )->pos );
				const vec3d pt1 = Trunc<4, 1>( rm.GetVertex( vb, i1 )->pos );
				const vec3d pt2 = Trunc<4, 1>( rm.GetVertex( vb, i2 )->pos );

				const vec3d tangent = ( pt1 - pt0 ).Normalize();
				const vec3d bitangent = ( pt2 - pt0 ).Normalize();
				const vec3d normal = Cross( tangent, bitangent ).Normalize();

				interpretedNormal += normal;
				interpretedTangent += tangent;
				interpretedBitangent += bitangent;
			}

			interpretedNormal.FlushDenorms();
			interpretedTangent.FlushDenorms();
			interpretedBitangent.FlushDenorms();

			vertex->normal = interpretedNormal.Normalize();
			vertex->tangent = interpretedTangent.Normalize();
			vertex->bitangent = interpretedBitangent.Normalize();
		}
	}
	else
	{
		// TODO: fix normals on this path
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
		/*
		v0.normal = vec3d( 1.0, 0.0, 0.0 );
		v1.normal = vec3d( 0.0, 1.0, 0.0 );
		v2.normal = vec3d( 0.0, 0.0, 1.0 );
		*/
		outInstance->triCache.push_back( Triangle( v0, v1, v2 ) );
	}

	if( material == nullptr )
	{
		outInstance->material = model->material;
	}
	else
	{
		outInstance->material = *material;
	}

	outInstance->BuildAS();
}


uint32_t CreatePlaneModel( const uint32_t vb, const uint32_t ib, const vec2d& size, const vec2i& cellCnt )
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

	vec2d gridSize = Divide( size, vec2d( cellCnt[ 0 ], cellCnt[ 1 ] ) );

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

	// Set material
	{
		model->material.Ka = 1.0;
		model->material.Tf = 1.0;
		model->material.Kd = 1.0;
		model->material.Ks = 1.0;
		model->material.Tr = 1.0;
		model->material.textured = false;
	}

	return modelIx;
}