#include"bitmap.h"

Bitmap::Bitmap( const std::string& filename )
{
	Load( filename );
}


Bitmap::~Bitmap()
{
	delete[] mapdata;
}


Bitmap::Bitmap( const Bitmap& bitmap )
{
	h = bitmap.h;

	pixelsNum = bitmap.pixelsNum;

	mapdata = new Pixel[ pixelsNum ];

	for ( uint32_t i = 0; i < pixelsNum; ++i )
	{
		mapdata[ i ] = bitmap.mapdata[ i ];
	}
}


Bitmap::Bitmap( const uint32_t width, const uint32_t height, const uint32_t color )
{
	pixelsNum = ( width * height );

	mapdata = new Pixel[ pixelsNum ];

	h.magicNum[ 0 ]	= 'B';
	h.magicNum[ 1 ]	= 'M';

	h.size			= ( pixelsNum * 4 );
	h.reserve1		= 0;
	h.reserve2		= 0;
	h.offset		= 54;

	h.hSize			= 40;
	h.width			= width;
	h.height		= height;
	h.cPlanes		= 1;
	h.bpPixels		= 32;
	h.compression	= 0;
	h.imageSize		= ( h.bpPixels * pixelsNum );
	h.hRes			= 0;
	h.vRes			= 0;
	h.colors		= 0;
	h.iColors		= 0;

	for ( size_t i = 0; i < pixelsNum; ++i )
	{
		mapdata[ i ].rgba = Color( color ).AsRGBA();
	}
}


Bitmap& Bitmap::operator=( const Bitmap& bitmap )
{
	return *this;
}


template<int ByteCnt>
void Bitmap::WriteInt( uint32_t value )
{
	assert( ByteCnt <= 4 );

	uint32_t temp = value;
	uint8_t bin[ ByteCnt ];
	
	for ( int32_t i = 0; i < ByteCnt; ++i )
	{
		bin[ i ] = static_cast<uint8_t>( temp & 0xFF );
		temp >>= 8;
	}

	outstream.write( reinterpret_cast<char*>( bin ), ByteCnt );
}


template<int ByteCnt>
uint32_t Bitmap::ReadInt()
{
	assert( ByteCnt <= 4 );

	uint8_t bin[ ByteCnt ];

	instream.read( reinterpret_cast<char*>( bin ), ByteCnt );

	const int cbyte = 8;
	uint32_t numeral = 0;

	for ( uint32_t i = 1; i <= ByteCnt; ++i )
	{
		uint32_t temp = static_cast<uint32_t>( bin[ ByteCnt - i ] );
		temp &= 0x000000FF;
		temp <<= ( ByteCnt - i ) * cbyte;
		numeral |= temp;
	}

	return numeral;
}


void Bitmap::Load( const std::string& filename )
{
	instream.open( filename.c_str(), std::ios::in | std::ios::binary );

	instream.read( reinterpret_cast<char*>( h.magicNum ), 2 );

	h.size			= ReadInt<4>();
	h.reserve1		= ReadInt<2>();
	h.reserve2		= ReadInt<2>();
	h.offset		= ReadInt<4>();

	h.hSize			= ReadInt<4>();
	h.width			= ReadInt<4>();
	h.height		= ReadInt<4>();
	h.cPlanes		= ReadInt<2>();
	h.bpPixels		= ReadInt<2>();
	h.compression	= ReadInt<4>();
	h.imageSize		= ReadInt<4>();
	h.hRes			= ReadInt<4>();
	h.vRes			= ReadInt<4>();
	h.colors		= ReadInt<4>();
	h.iColors		= ReadInt<4>();

	uint32_t padding = 0;

	if ( ( h.width % 4 ) != 0 )
	{
		padding = 4 - ( h.width % 4 );
	}

	instream.seekg( h.offset, std::ios_base::beg );

	uint32_t bytes = h.imageSize;

	mapdata = new Pixel[ bytes ];

	const int32_t pixelBytes = ( h.bpPixels / 8 );
	const int32_t lineBytes = ( pixelBytes * h.width );
	uint8_t* buffer = new uint8_t[ lineBytes ];

	for ( int32_t row = ( h.height - 1 ); row >= 0; row-- )
	{
		instream.read( reinterpret_cast<char*>( buffer ), lineBytes );

		for ( uint32_t pixNum = 0; pixNum < ( h.width * pixelBytes ); pixNum += pixelBytes )
		{
			RGBA pixel;
			//parse raw data width/24bits
			pixel.b	= buffer[ pixNum ];
			pixel.g	= buffer[ pixNum + 1 ];
			pixel.r	= buffer[ pixNum + 2 ];
			pixel.a	= (uint8_t) 0xFF;

			mapdata[ row * h.width + ( pixNum / pixelBytes ) ].rgba = pixel;
		}

		instream.seekg( padding + instream.tellg(), std::ios_base::beg );
	}

	delete[] buffer;
	buffer = nullptr;

	instream.close();
}


void Bitmap::Write( const std::string& filename )
{
	outstream.open( filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc );

	outstream.write( reinterpret_cast<char*>( h.magicNum ), 2 );

	WriteInt<4>( h.size );
	WriteInt<2>( h.reserve1 );
	WriteInt<2>( h.reserve2 );
	WriteInt<4>( h.offset );

	WriteInt<4>( h.hSize );
	WriteInt<4>( h.width );
	WriteInt<4>( h.height );
	WriteInt<2>( h.cPlanes );
	WriteInt<2>( h.bpPixels );
	WriteInt<4>( h.compression );
	WriteInt<4>( h.imageSize );
	WriteInt<4>( h.hRes );
	WriteInt<4>( h.vRes );
	WriteInt<4>( h.colors );
	WriteInt<4>( h.iColors );

	outstream.seekp( h.offset, std::ios_base::beg );

	int padding = 0;
	if ( ( h.width % 4 ) != 0 )
	{
		padding = 4 - ( h.width % 4 );
	}

	const uint8_t pad[ 4 ] = { '\0','\0', '\0','\0' };

	for ( int32_t row = ( h.height - 1 ); row >= 0; row-- )
	{
		const int32_t rowOffset = ( row * h.width );
		for ( uint32_t colIx = 0; colIx < h.width; colIx++ )
		{
			Pixel pixel;
			CopyToPixel( mapdata[ colIx + rowOffset ].rgba, pixel, BITMAP_ARGB );
			WriteInt<4>( pixel.r8g8b8a8 );
			// xARGB
		}

		outstream.write( reinterpret_cast<const char*>( pad ), padding );
	}

	outstream.close();
}


uint32_t Bitmap::GetSize()
{
	return h.imageSize;
}


uint32_t Bitmap::GetWidth()
{
	return h.width;
}


uint32_t Bitmap::GetHeight()
{
	return h.height;
}


void Bitmap::ClearImage()
{
	for ( uint32_t i = 0; i < h.imageSize; ++i )
	{
		mapdata[ i ].r8g8b8a8 = 0;
	}
}


void Bitmap::GetBuffer( uint32_t buffer[] )
{
	for ( uint32_t i = 0; i < pixelsNum; ++i )
	{
		Pixel pixel;
		CopyToPixel( mapdata[ i ].rgba, pixel, BITMAP_BGRA );

		buffer[ i ] = pixel.r8g8b8a8;
	}
}


bool Bitmap::SetPixel( const int32_t x, const int32_t y, const uint32_t color )
{
	if ( ( x >= static_cast<int32_t>( h.width ) ) || ( x < 0 ) || ( y >= static_cast<int32_t>( h.height ) ) || ( y < 0 ) )
	{
		return false;
	}
	else
	{
		mapdata[ y * h.width + x ].rgba = Color( color ).AsRGBA();
		return true;
	}
}


uint32_t Bitmap::GetPixel( const int32_t x, const int32_t y )
{
	if ( ( x >= static_cast<int32_t>( h.width ) ) || ( x < 0 ) || ( y >= static_cast<int32_t>( h.height ) ) || ( y < 0 ) )
	{
		return 0;
	}
	else
	{
		return mapdata[ y * h.width + x ].r8g8b8a8;
	}
}


void Bitmap::CopyToPixel( const RGBA& rgba, Pixel& pixel, BitmapFormat format )
{
	switch ( format )
	{
	case BITMAP_ABGR:
	{
		pixel[ 0 ] = rgba.a;
		pixel[ 1 ] = rgba.b;
		pixel[ 2 ] = rgba.g;
		pixel[ 3 ] = rgba.r;
	}
	break;

	case BITMAP_ARGB:
	{
		pixel[ 0 ] = rgba.a;
		pixel[ 1 ] = rgba.r;
		pixel[ 2 ] = rgba.g;
		pixel[ 3 ] = rgba.b;
	}
	break;

	case BITMAP_BGRA:
	{
		pixel[ 0 ] = rgba.b;
		pixel[ 1 ] = rgba.g;
		pixel[ 2 ] = rgba.r;
		pixel[ 3 ] = rgba.a;
	}
	break;

	default:
	case BITMAP_RGBA:
	{
		pixel[ 0 ] = rgba.r;
		pixel[ 1 ] = rgba.g;
		pixel[ 2 ] = rgba.b;
		pixel[ 3 ] = rgba.a;
	}
	break;
	}
}