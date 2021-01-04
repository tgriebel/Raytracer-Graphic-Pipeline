#pragma once

#include <assert.h>
#include "common.h"

template<typename T>
class Image
{
public:

	Image() : ~Image()
	{
		
	}

	Image( const uint32_t _width, const uint32_t _height, const T _default = static_cast<T>( 0.0 ), const char* _name = "" )
	{
		name = _name;

		width = _width;
		height = _height;
		length = width * height;
		buffer = new T[ length ];

		Clear( _default );
	}

	~Image()
	{
		width = 0;
		height = 0;
		length = 0;
		delete[] buffer;
		name = "";
	}

	Image& operator=( const Image& _image )
	{
		for ( uint32_t i = 0; i < _image.length; ++i )
		{
			buffer[ i ] = _image.buffer[ i ];
		}

		width = _image.width;
		height = _image.height;
		length = _image.length;
		name = _image.name;

		return *this;
	}

	bool SetPixel( const int32_t x, const int32_t y, const T& pixel )
	{
		if ( ( x >= static_cast<int32_t>( width ) ) || ( x < 0 ) || ( y >= static_cast<int32_t>( height ) ) || ( y < 0 ) )
			return false;

		const uint32_t index = ( x + y * width );
		assert( index < length );

		if ( index >= length )
			return false;

		buffer[ index ] = pixel;
		return true;
	}

	T GetPixel( const int32_t x, const int32_t y )
	{
		if ( ( x >= static_cast<int32_t>( width ) ) || ( x < 0 ) || ( y >= static_cast<int32_t>( height ) ) || ( y < 0 ) )
			return static_cast<T>( 0 );

		const uint32_t index = ( x + y * width );
		assert( index < length );

		if ( index >= length )
			return static_cast<T>( 0 );

		return buffer[ index ];
	}

	void Clear( const T& fill )
	{
		for ( uint32_t i = 0; i < length; ++i )
		{
			buffer[ i ] = fill;
		}
	}

	inline const T* const GetRawBuffer() const
	{
		return &buffer[ 0 ];
	}

	inline uint32_t GetWidth() const
	{
		return width;
	}

	inline uint32_t GetHeight() const
	{
		return height;
	}

	inline uint32_t GetByteCnt() const
	{
		return length * sizeof( T );
	}

	inline const char* GetName() const
	{
		return name;
	}

private:
	uint32_t	width;
	uint32_t	height;
	uint32_t	length;
	T*			buffer;
	const char*	name;
};