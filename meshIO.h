
#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>

namespace GeomReader
{
	class Polytope;
	class Tetgen;
	class Off;
	class Obj;

	void RemoveUnnecessaryCharacters( std::fstream& stream, std::stringstream& newStream );

	void ReadTetGen( const std::string& baseFileName, Tetgen& mesh );
	void WriteTetGen( const std::string& baseFileName, const Tetgen& mesh );

	void ReadOFF( const std::string& fileName, Off& mesh );
	void WriteOFF( const std::string& fileName, const Off& mesh );

	// void ReadObj( string fileName, Obj& mesh );
	// void WriteObj( string fileName, const Obj& mesh );

	struct node_t
	{
		node_t() : x( 0 ), y( 0 ), z( 0 ), r( 0 ), g( 0 ), b( 0 ), a( 0 ) {}

		double	x, y, z;
		double	r, g, b, a;
		bool	colored;
	};

	struct edge_t {

		int32_t n1, n2;
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
		int32_t nvertices, nedges, nfaces;
		node_t* vertices;
		Polytope* faces;
		bool vertices_colored, faces_colored;

		Off() :nvertices( 0 ), nedges( 0 ), nfaces( 0 ), vertices( NULL ), faces( NULL ), vertices_colored( false ), faces_colored( false ) {}

		Off( int32_t _vertices, int32_t _edges, int32_t _faces ) : nvertices( _vertices ), nedges( _edges ), nfaces( _faces ), vertices_colored( false ), faces_colored( false )
		{
			vertices = new node_t[ nvertices ];
			faces = new Polytope[ nfaces ];
		}

		Off( const Off& source ) :
			nvertices( source.nvertices ),
			nedges( source.nedges ),
			nfaces( source.nfaces ),
			vertices_colored( source.vertices_colored ),
			faces_colored( source.faces_colored )
		{
			vertices = new node_t[ nvertices ];
			faces = new Polytope[ nfaces ];

			for ( int32_t i( 0 ); i < nvertices; ++i )	vertices[ i ] = source.vertices[ i ];
			for ( int32_t i( 0 ); i < nfaces; ++i )		faces[ i ] = source.faces[ i ];
		}

		void Init( int32_t _vertices, int32_t _edges, int32_t _faces )
		{
			nvertices = _vertices;
			nedges = _edges;
			nfaces = _faces;
			vertices_colored = false;
			faces_colored = false;

			vertices = new node_t[ nvertices ];
			faces = new Polytope[ nfaces ];
		}

		~Off()
		{
			delete[] vertices;
			delete[] faces;
		}
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


	static void RemoveUnnecessaryCharacters( std::fstream& stream, std::stringstream& new_stream )
	{
		std::vector<std::string> lines;
		std::string tmp_line;

		while ( !stream.eof() ) {
			getline( stream, tmp_line );

			//TODO: find '#' and remove what follows, instead of checking first char
			if ( tmp_line.size() > 0 && tmp_line[ 0 ] != '#' && tmp_line != "\n" ) {
				lines.push_back( tmp_line + "\n" );
			}
		}

		std::string buf;
		for ( std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it ) {

			buf += *it;
		}

		new_stream.str( buf );
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
			nodeStream >> mesh.vertices[ i ].x;
			nodeStream >> mesh.vertices[ i ].y;
			nodeStream >> mesh.vertices[ i ].z;
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
			nodeFileStream << ( i + 1 ) << " " << mesh.vertices[ i ].x << " " << mesh.vertices[ i ].y << " " << mesh.vertices[ i ].z << " \n";
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
		for ( int32_t i = 0; i < mesh.nvertices; ++i )
		{
			getline( inputStream, line );
			ss.clear();
			ss.str( line );

			ss >> mesh.vertices[ i ].x;
			ss >> mesh.vertices[ i ].y;
			ss >> mesh.vertices[ i ].z;

			ss >> mesh.vertices[ i ].r;
			ss >> mesh.vertices[ i ].g;
			ss >> mesh.vertices[ i ].b;
			ss >> mesh.vertices[ i ].a;

			mesh.vertices[ i ].colored = true;
		}

		// Read polygons
		for ( int32_t i = 0; i < mesh.nfaces; ++i )
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
		output << mesh.nvertices << " " << mesh.nfaces << " " << mesh.nedges << "\n";

		// Write vertices
		for ( int32_t i = 0; i < mesh.nvertices; ++i )
		{
			output << mesh.vertices[ i ].x << " " << mesh.vertices[ i ].y << " " << mesh.vertices[ i ].z;

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
		for ( int32_t i = 0; i < mesh.nfaces; ++i )
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

	/*
	void WriteObj( const string fileName, const Obj& mesh )
	{
		fstream file;
		file.open( fileName, std::fstream::out );

		for ( int i = 0; i < length; ++i )
		{
			file << "v " << vertList[ i ].pos[ 0 ] << " " << vertList[ i ].pos[ 1 ] << " " << vertList[ i ].pos[ 2 ] << "\n";
		}

		for ( int i = 0; i < TriCnt; ++i ) {

			vec3d b1 = vec3d( triList[ i ].v1 - triList[ i ].v0 );
			vec3d b2 = vec3d( triList[ i ].v2 - triList[ i ].v0 );

			vec3d norm = Cross( b1, b2 );

			file << "fn " << norm[ 0 ] << " " << norm[ 1 ] << " " << norm[ 2 ] << "\n";
		}

		for ( int i = 0; i < TriCnt; ++i ) {

			file << "f " << ( i * 3 + 1 ) << " " << ( i * 3 + 1 + 1 ) << " " << ( i * 3 + 2 + 1 ) << "\n";
		}
	}
	*/
};