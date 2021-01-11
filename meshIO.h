
#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <map>
#include <iomanip>

namespace MeshIO
{
	class Polytope;
	class Tetgen;
	class Off;
	class Obj;

	static inline void CleanLine( std::fstream& stream, std::stringstream& newStream );

	static inline void ReadTetGen( const std::string& baseFileName, Tetgen& mesh );
	static inline void WriteTetGen( const std::string& baseFileName, const Tetgen& mesh );

	static inline void ReadOFF( const std::string& fileName, Off& mesh );
	static inline void WriteOFF( const std::string& fileName, const Off& mesh );

	static inline void ReadObj( const std::string& fileName, Obj& mesh );
	static inline void WriteObj( const std::string& fileName, const Obj& mesh );

	struct vector_t
	{
		vector_t() : x( 0.0 ), y( 0.0 ), z( 0.0 ), w( 0.0 ) {}

		double	x;
		double	y;
		double	z;
		double	w;
	};

	struct node_t
	{
		node_t() : r( 0 ), g( 0 ), b( 0 ), a( 0 ) {}

		vector_t pos;
		double	r;
		double	g;
		double	b;
		double	a;
		bool	colored;
	};

	struct objIndex_t
	{
		int32_t vertexIx;
		int32_t uvIx;
		int32_t normalIx;		
	};

	struct objFace_t
	{
		std::vector<objIndex_t> vertices;
	};

	struct objSmoothingGroup_t
	{
		std::vector<objFace_t> faces;
	};

	struct objGroup_t
	{
		std::string material;
		std::map<int32_t, objSmoothingGroup_t> smoothingGroups;
	};

	struct edge_t
	{
		int32_t	n1;
		int32_t	n2;
		edge_t() : n1( 0 ), n2( 0 ) {}
	};

	class Polytope
	{
	public:
		int32_t		npoints;
		int32_t*	points;
		double		r, g, b, a;
		bool		planar;

		Polytope() : npoints( 0 ), points( NULL ), r( 0 ), g( 0 ), b( 0 ), a( 0 ), planar( true ) {}

		Polytope( int32_t _npoints, bool _planar ) : npoints( _npoints ), r( 0 ), g( 0 ), b( 0 ), a( 0 ), planar( _planar )
		{
			points = new int32_t[ npoints ];
		}

		Polytope( const Polytope& source ) :
			npoints( source.npoints ),
			r( source.r ),
			g( source.g ),
			b( source.b ),
			a( source.a ),
			planar( source.planar )
		{
			points = new int32_t[ npoints ];
			for ( int32_t i( 0 ); i < npoints; ++i )
				points[ i ] = source.points[ i ];
		}

		void init( int32_t _npoints, bool _planar )
		{
			r = g = b = a = 0;
			planar = _planar;
			npoints = _npoints;
			points = new int32_t[ _npoints ];
		}

		~Polytope()
		{
			if ( points != NULL )
			{
				delete[] points;
			}
		}
	};

	class Off
	{
	public:
		int32_t		verticesCnt;
		int32_t		edgesCnt;
		int32_t		facesCnt;
		node_t*		vertices;
		Polytope*	faces;
		bool		vertices_colored;
		bool		faces_colored;

		Off() :verticesCnt( 0 ), edgesCnt( 0 ), facesCnt( 0 ), vertices( NULL ), faces( NULL ), vertices_colored( false ), faces_colored( false ) {}

		Off( int32_t _vertices, int32_t _edges, int32_t _faces ) : verticesCnt( _vertices ), edgesCnt( _edges ), facesCnt( _faces ), vertices_colored( false ), faces_colored( false )
		{
			vertices = new node_t[ verticesCnt ];
			faces = new Polytope[ facesCnt ];
		}

		Off( const Off& source ) :
			verticesCnt( source.verticesCnt ),
			edgesCnt( source.edgesCnt ),
			facesCnt( source.facesCnt ),
			vertices_colored( source.vertices_colored ),
			faces_colored( source.faces_colored )
		{
			vertices = new node_t[ verticesCnt ];
			faces = new Polytope[ facesCnt ];

			for ( int32_t i( 0 ); i < verticesCnt; ++i )	vertices[ i ] = source.vertices[ i ];
			for ( int32_t i( 0 ); i < facesCnt; ++i )		faces[ i ] = source.faces[ i ];
		}

		void Init( int32_t _vertices, int32_t _edges, int32_t _faces )
		{
			verticesCnt = _vertices;
			edgesCnt = _edges;
			facesCnt = _faces;
			vertices_colored = false;
			faces_colored = false;

			vertices = new node_t[ verticesCnt ];
			faces = new Polytope[ facesCnt ];
		}

		~Off()
		{
			delete[] vertices;
			delete[] faces;
		}
	};

	class Obj
	{
	public:
		std::vector<std::string>			materialLibs;
		std::vector<vector_t>				vertices;
		std::vector<vector_t>				normals;
		std::vector<vector_t>				uvs;
		std::map<std::string, objGroup_t>	groups;
	};


	class Tetgen
	{
	public:
		int32_t numVertices, numFaces, numTets;

		node_t* vertices;
		Polytope* faces;
		Polytope* tets;

		Tetgen( int32_t n_vertices_i, int32_t n_faces_i, int32_t n_tets_i ) :
			numVertices( n_vertices_i ),
			numFaces( n_faces_i ),
			numTets( n_tets_i ),
			vertices( NULL ),
			faces( NULL ),
			tets( NULL )
		{
			if ( numVertices > 0 )	vertices = new node_t[ numVertices ];
			if ( numFaces > 0 )		faces = new Polytope[ numFaces ];
			if ( numTets > 0 )		tets = new Polytope[ numFaces ];
		}

		Tetgen() : vertices( NULL ), faces( NULL ), tets( NULL ), numVertices( 0 ), numFaces( 0 ), numTets( 0 ) {}

		~Tetgen()
		{
			if ( vertices != NULL )	delete[] vertices;
			if ( faces != NULL )	delete[] faces;
			if ( tets != NULL )		delete[] tets;
		}

		void Init( int32_t _numVertices, int32_t _numFaces, int32_t _numTets )
		{
			numVertices = _numVertices;
			numFaces = _numFaces;
			numTets = _numTets;

			if ( vertices != NULL )	delete[] vertices;
			if ( faces != NULL )	delete[] faces;
			if ( tets != NULL )		delete[] tets;

			if ( numVertices > 0 )	vertices = new node_t[ numVertices ];
			if ( numFaces > 0 )		faces = new Polytope[ numFaces ];
			if ( numTets > 0 )		tets = new Polytope[ numTets ];
		}

		void InitVertices( int32_t _numVertices )
		{
			if ( _numVertices <= 0 )
				return;

			numVertices = _numVertices;
			if ( vertices != NULL )
			{
				delete[] vertices;
			}
			vertices = new node_t[ numVertices ];
		}

		void InitFaces( int32_t _numFaces )
		{
			if ( _numFaces <= 0 )
				return;

			numFaces = _numFaces;
			if ( faces != NULL )
			{
				delete[] faces;
			}
			faces = new Polytope[ numFaces ];
		}

		void InitTets( int32_t _numTets )
		{
			if ( _numTets <= 0 )
				return;

			numTets = _numTets;
			if ( tets != NULL )
			{
				delete[] tets;
			}
			tets = new Polytope[ numTets ];
		}
	};


	static void CleanLine( std::string& str, std::string& outStr )
	{
		const size_t offset = str.find( "#" );

		if( offset != std::string::npos )
		{
			outStr = str.substr( 0, offset );
			return;
		}

		outStr = str;

		std::for_each( outStr.begin(), outStr.end(), []( char& c ) {
			c = ::tolower( c );
		} );
	}

	void ReadTetGen( const std::string& baseFileName, Tetgen& mesh )
	{
		std::string nodeFileName = baseFileName + ".node";
		std::fstream nodeStream( nodeFileName.c_str(), std::fstream::in );

		if ( !nodeStream )
			return;

		//First line: <# of points> <dimension (must be 3)> <# of attributes> <# of boundary markers (0 or 1)>
		//Remaining lines list # of points:
		//<point #> <x> <y> <z> [attributes] [boundary marker]

		// TODO: do something with attributes and markers??
		int32_t trash;
		int32_t points;
		nodeStream >> points;
		nodeStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		
		// Read vertices
		mesh.InitVertices( points );
		for ( int32_t i = 0; i < points; ++i )
		{
			nodeStream >> trash;
			nodeStream >> mesh.vertices[ i ].pos.x;
			nodeStream >> mesh.vertices[ i ].pos.y;
			nodeStream >> mesh.vertices[ i ].pos.z;
			nodeStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		}
		nodeStream.close();

		//First line: <# of faces> <boundary marker (0 or 1)>
		//Remaining lines list of # of faces:
		//<face #> <node> <node> <node> [boundary marker]

		std::string face_file_name = baseFileName + ".face";
		std::fstream faceStream( face_file_name.c_str(), std::fstream::in );
		if ( !faceStream ) return;

		int32_t faces;
		faceStream >> faces;
		faceStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		
		// Read faces
		mesh.InitFaces( faces );
		for ( int32_t i( 0 ); i < faces; ++i )
		{
			faceStream >> trash;
			mesh.faces[ i ].init( 3, true );
			faceStream >> mesh.faces[ i ].points[ 0 ];
			faceStream >> mesh.faces[ i ].points[ 1 ];
			faceStream >> mesh.faces[ i ].points[ 2 ];
			faceStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		}
		faceStream.close();

		//First line: <# of tetrahedra> <nodes per tetrahedron> <# of attributes>
		//Remaining lines list of # of tetrahedra:
		//<tetrahedron #> <node> <node> <node> <node> ... [attributes]

		std::string elementFileName = baseFileName + ".ele";
		std::fstream elementStream = std::fstream( elementFileName.c_str(), std::fstream::in );
		if ( !elementStream )
			return;

		int32_t tets, nodes_per;
		elementStream >> tets;
		elementStream >> nodes_per;
		elementStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		//read tets

		mesh.InitTets( tets );
		for ( int32_t i = 0; i < tets; ++i )
		{
			elementStream >> trash;

			mesh.tets[ i ].init( nodes_per, false );
			for ( int32_t j = 0; j < nodes_per; ++j )
			{
				elementStream >> mesh.tets[ i ].points[ j ];
			}
			elementStream.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
		}
		elementStream.close();
	}


	void WriteTetGen( const std::string& baseFileName, const Tetgen& mesh )
	{
		std::fstream nodeFileStream		= std::fstream( ( baseFileName + ".node" ).c_str(), std::fstream::out );
		std::fstream faceFileStream		= std::fstream( ( baseFileName + ".face" ).c_str(), std::fstream::out );
		std::fstream elementFileStream	= std::fstream( ( baseFileName + ".ele" ).c_str(), std::fstream::out );

		//First line: <# of points> <dimension (must be 3)> <# of attributes> <# of boundary markers (0 or 1)>
		//Remaining lines list # of points:
		//<point #> <x> <y> <z> [attributes] [boundary marker]

		nodeFileStream << mesh.numVertices << " " << 3 << " " << 0 << " " << 0 << "\n";

		// Write vertices
		for ( int32_t i = 0; i < mesh.numVertices; ++i )
		{
			nodeFileStream << ( i + 1 ) << " " << mesh.vertices[ i ].pos.x << " " << mesh.vertices[ i ].pos.y << " " << mesh.vertices[ i ].pos.z << " \n";
		}
		nodeFileStream << "#finished" << std::endl;
		nodeFileStream.close();
		//First line: <# of faces> <boundary marker (0 or 1)>
		//Remaining lines list of # of faces:
		//<face #> <node> <node> <node> [boundary marker]

		// Write faces
		faceFileStream << mesh.numFaces << " " << 1 << "\n";

		for ( int32_t i = 0; i < mesh.numFaces; ++i )
		{
			faceFileStream << ( i + 1 ) << " " << mesh.faces[ i ].points[ 0 ] << " " << mesh.faces[ i ].points[ 1 ] << " " << mesh.faces[ i ].points[ 2 ] << "\n";
		}
		faceFileStream << "#finished" << std::endl;
		faceFileStream.close();

		//First line: <# of tetrahedra> <nodes per tetrahedron> <# of attributes>
		//Remaining lines list of # of tetrahedra:
		//<tetrahedron #> <node> <node> <node> <node> ... [attributes]

		// Write tets
		elementFileStream << mesh.numTets << " " << 4 << " " << 0 << "\n";

		for ( int32_t i = 0; i < mesh.numTets; ++i )
		{
			elementFileStream << ( i + 1 ) << " ";

			for ( int32_t j = 0; j < 4; ++j )
			{
				elementFileStream << mesh.tets[ i ].points[ j ] << " ";
			}
			elementFileStream << "\n";
		}
		elementFileStream << "#finished" << std::endl;
		elementFileStream.close();
	}


	void ReadOFF( const std::string& file_name, Off& mesh )
	{
		// TODO: close if not exist!!

		std::fstream inputStream = std::fstream( file_name.c_str(), std::fstream::in );

		//stringstream input_stream;
		//_removeUnnecessaryCharacters(filestr, input_stream);

		std::string line( "" );
		getline( inputStream, line );

		getline( inputStream, line );

		std::stringstream ss;
		ss.clear();
		ss.str( line );

		int32_t vertices, edges, faces;
		ss >> vertices;
		ss >> faces;
		ss >> edges;

		mesh.Init( vertices, edges, faces );

		// Read vertices
		for ( int32_t i = 0; i < mesh.verticesCnt; ++i )
		{
			getline( inputStream, line );
			ss.clear();
			ss.str( line );

			ss >> mesh.vertices[ i ].pos.x;
			ss >> mesh.vertices[ i ].pos.y;
			ss >> mesh.vertices[ i ].pos.z;

			ss >> mesh.vertices[ i ].r;
			ss >> mesh.vertices[ i ].g;
			ss >> mesh.vertices[ i ].b;
			ss >> mesh.vertices[ i ].a;

			mesh.vertices[ i ].colored = true;
		}

		// Read polygons
		for ( int32_t i = 0; i < mesh.facesCnt; ++i )
		{
			getline( inputStream, line );
			ss.clear();
			ss.str( line );

			int32_t pts;
			ss >> pts;
			mesh.faces[ i ].init( pts, true );

			for ( int32_t j = 0; j < mesh.faces[ i ].npoints; ++j )
			{
				int32_t buffer;
				ss >> buffer;
				mesh.faces[ i ].points[ j ] = buffer;
			}
		}
		/*
		//read face colors
		if(!input_stream.eof()){
			for(SIZE i(0); i < mesh.nvertices; ++i){

				getline(input_stream, line);
				ss.clear();
				ss.str(line);

				SIZE trash;
				for(SIZE j(0); j <= mesh.faces[i].npoints; ++j){
					ss >> trash;
				}

				ss >> mesh.faces[i].r;
				ss >> mesh.faces[i].g;
				ss >> mesh.faces[i].b;
				ss >> mesh.faces[i].a;
			}

			mesh.faces_colored = true;
		}*/

		inputStream.close();
	}


	void WriteOFF( const std::string& file_name, const Off& mesh )
	{
		std::fstream output = std::fstream( file_name.c_str(), std::fstream::out );
		output << "OFF" << "\n";
		output << mesh.verticesCnt << " " << mesh.facesCnt << " " << mesh.edgesCnt << "\n";

		// Write vertices
		for ( int32_t i = 0; i < mesh.verticesCnt; ++i )
		{
			output << mesh.vertices[ i ].pos.x << " " << mesh.vertices[ i ].pos.y << " " << mesh.vertices[ i ].pos.z;

			if ( mesh.vertices[ i ].colored )
			{
				output << " " << mesh.vertices[ i ].r << " " << mesh.vertices[ i ].g << " " << mesh.vertices[ i ].b << " " << mesh.vertices[ i ].a << "\n";
			}
			else
			{
				output << " " << 1 << " " << 1 << " " << 1 << " " << 1 << "\n";
			}
		}

		// Write faces
		for ( int32_t i = 0; i < mesh.facesCnt; ++i )
		{
			output << mesh.faces[ i ].npoints << " ";

			for ( int32_t j = 0; j < mesh.faces[ i ].npoints; ++j )
			{
				output << mesh.faces[ i ].points[ j ] << " ";
			}

			output << "\n";
		}

		output.close();
	}

	void ReadObj( const std::string& file_name, Obj& mesh )
	{
		std::fstream inputStream = std::fstream( file_name.c_str(), std::fstream::in );

		struct state_t
		{
			std::string	object;
			std::string	group;
			std::string	material;
			int32_t		smoothGroup;
		};

		state_t state;
		state.object = "";
		state.group = "";
		state.material = "";
		state.smoothGroup = 0;

		mesh.vertices.reserve( 10000 );
		mesh.normals.reserve( 10000 );
		mesh.uvs.reserve( 10000 );

		while( !inputStream.eof() )
		{
			std::string line = "";
			std::string srcLine = "";

			getline( inputStream, srcLine );

			CleanLine( srcLine, line );

			if( line.length() == 0 )
				continue;

			std::stringstream ss;
			ss.clear();
			ss.str( line );

			std::string semanticToken;
			ss >> semanticToken;

			if( semanticToken.compare( "mtllib") == 0 )
			{
				std::string mtlFileName;
				ss >> mtlFileName;
				mesh.materialLibs.push_back( mtlFileName );
			}
			else if ( semanticToken.compare( "v" ) == 0 )
			{
				vector_t v;
				ss >> v.x;
				ss >> v.y;
				ss >> v.z;
				mesh.vertices.push_back( v );
			}
			else if ( semanticToken.compare( "vn" ) == 0 )
			{
				vector_t v;
				ss >> v.x;
				ss >> v.y;
				ss >> v.z;
				mesh.normals.push_back( v );
			}
			else if ( semanticToken.compare( "vt" ) == 0 ) // Texture UVs
			{
				vector_t v;
				ss >> v.x;
				if( !ss.eof() )
				{
					ss >> v.y;
				}
				if ( !ss.eof() )
				{
					ss >> v.z;
				}
				mesh.uvs.push_back( v );
			}
			else if ( semanticToken.compare( "vp" ) == 0 ) // Parameter space vertices
			{
				assert( false ); // Unimplemented
			}
			else if ( semanticToken.compare( "s" ) == 0 ) // Smooth shading
			{
				ss >> state.smoothGroup;
			}
			else if ( semanticToken.compare( "o" ) == 0 ) // Object name
			{
				assert( false ); // Unimplemented
				ss >> state.object;
			}
			else if ( semanticToken.compare( "g" ) == 0 ) // Group name
			{
				ss >> state.group;
			}
			else if ( semanticToken.compare( "usemtl" ) == 0 ) // Material for element
			{
				ss >> state.material;
				mesh.groups[ state.group ].material = state.material;
			}
			else if ( semanticToken.compare( "f" ) == 0 ) // Faces
			{
				objFace_t face;

				while( !ss.eof() )
				{
					objIndex_t indices;
					indices.vertexIx = -1;
					indices.uvIx = -1;
					indices.normalIx = -1;

					std::string facePt;
					ss >> facePt;
					std::stringstream vertexStream( facePt );
					
					if( !vertexStream.rdbuf()->in_avail() )
						break;

					int32_t elementIx = 0;

					while( !vertexStream.eof() && ( elementIx < 3 ) )
					{
						std::string token;
						std::getline( vertexStream, token, '/' );

						std::stringstream tokenStream;
						tokenStream.clear();
						tokenStream.str( token );

						if( tokenStream.eof() || ( token.length() == 0 ) )
						{
							++elementIx;
							break;
						}

						int32_t index;
						tokenStream >> index;

						index -= 1;

						if( elementIx == 0 )
						{
							indices.vertexIx = index;
						} 
						else if( elementIx == 1 )
						{
							indices.uvIx = index;
						}
						else if ( elementIx == 2 )
						{
							indices.normalIx = index;
						}
						++elementIx;
					}					
					face.vertices.push_back( indices );
				}

				objGroup_t& group = mesh.groups[ state.group ];
				objSmoothingGroup_t& smoothingGroup = group.smoothingGroups[ state.smoothGroup ];
				smoothingGroup.faces.push_back( face );
			}
			else if ( semanticToken.compare( "l" ) == 0 ) // Lines
			{
				assert( false ); // Unimplemented
			}
		}

		inputStream.close();		
	}

	void WriteObj( const std::string& fileName, const Obj& mesh )
	{
		std::fstream file;
		file.open( fileName, std::fstream::out );

		file << std::fixed << std::showpoint << std::setprecision( 4 );

		file << "# MeshIO Export\n\n\n";

		const size_t vertCnt = mesh.vertices.size();
		for ( int i = 0; i < vertCnt; ++i )
		{
			file << "v  " << mesh.vertices[ i ].x << " " << mesh.vertices[ i ].y << " " << mesh.vertices[ i ].z << "\n";
		}
		file << "# " << vertCnt << " vertices\n\n";

		const size_t normalCnt = mesh.normals.size();
		for ( int i = 0; i < normalCnt; ++i )
		{
			file << "vn " << mesh.normals[ i ].x << " " << mesh.normals[ i ].y << " " << mesh.normals[ i ].z << "\n";
		}
		file << "# " << normalCnt << " vertex normals\n\n";

		const size_t uvCnt = mesh.uvs.size();
		for ( int i = 0; i < uvCnt; ++i )
		{
			file << "vt " << mesh.uvs[ i ].x << " " << mesh.uvs[ i ].y << " " << mesh.uvs[ i ].z << "\n";
		}
		file << "# " << uvCnt << " texture coords\n\n";

		auto groupIttEnd = mesh.groups.end();
		for ( auto groupItt = mesh.groups.begin(); groupItt != groupIttEnd; ++groupItt )
		{			
			file << "g " << groupItt->first << "\n";
			file << "usemtl " << groupItt->second.material << "\n";

			objGroup_t group = groupItt->second;

			auto smoothGroupIttEnd = group.smoothingGroups.end();
			for ( auto smoothGroupItt = group.smoothingGroups.begin(); smoothGroupItt != smoothGroupIttEnd; ++smoothGroupItt )
			{
				file << "s " << smoothGroupItt->first << "\n";

				objSmoothingGroup_t smoothGroup = smoothGroupItt->second;
				
				auto faceIttEnd = smoothGroup.faces.end();
				for ( auto faceItt = smoothGroup.faces.begin(); faceItt != faceIttEnd; ++faceItt )
				{
					file << "f ";

					auto vertIttEnd = faceItt->vertices.end();
					for ( auto vertItt = faceItt->vertices.begin(); vertItt != vertIttEnd; ++vertItt )
					{
						file << ( 1 + vertItt->vertexIx );

						if ( vertItt->uvIx != -1 )
						{
							file << "/" << ( 1 + vertItt->uvIx );
						}

						if( vertItt->normalIx != -1 )
						{
							file << "/" << ( 1 + vertItt->normalIx );
						}

						file << " ";
					}
					file << "\n";
				}
				file << "# " << uvCnt << " faces\n\n";
			}		
		}

		file.close();
	}
};