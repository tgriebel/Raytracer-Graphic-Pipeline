#pragma once

#include <assert.h>
#include <cstdint>
#include <algorithm>

#include "common.h"

template<typename T>
struct RgbaTuple
{
	T	a;
	T	b;
	T	g;
	T	r;
};
using RGBA = RgbaTuple<uint8_t>;


template<typename T>
struct RgbTuple
{
	T	b;
	T	g;
	T	r;
};


union Pixel
{
private:
	uint8_t		vec[ 5 ];
public:
	uint32_t	r8g8b8a8;
	RGBA		rgba;

	Pixel() : r8g8b8a8( 0 ) {}
	Pixel( const uint32_t _hexColor ) : r8g8b8a8( _hexColor ) {}
	Pixel( const RGBA& _rgba ) : rgba( rgba ) {}

	uint8_t& operator[]( const uint32_t i )
	{
		if ( i >= 4 )
		{
			assert( false );
			return vec[ 4 ];
		}

		return vec[ 3 - i ];
	}
};


enum class blendMode_t : uint32_t
{
	SRCALPHA,
	DESTALPHA,
	INVSRCALPHA,
	INVDESTALPHA,
	SRCCOLOR,
	DESTCOLOR,
	INVSRCCOLOR,
	INVDESTCOLOR,
	ZERO,
	ONE,
	ADD,
	SUBTRACT,
	REVSUBTRACT,
	MIN,
	MAX,
	XOR,
	COUNT,
};


class Color
{
public:
	static const uint32_t Red		= 0xFF0000FF;
	static const uint32_t Green		= 0x00FF00FF;
	static const uint32_t Blue		= 0x0000FFFF;
	static const uint32_t White		= 0xFFFFFFFF;
	static const uint32_t Black		= 0x000000FF;
	static const uint32_t LGrey		= 0xEEEEEEFF;
	static const uint32_t DGrey		= 0x333333FF;
	static const uint32_t Brown		= 0x111111FF;
	static const uint32_t Cyan		= 0x00FFFFFF;
	static const uint32_t Yellow	= 0xFFFF00FF;
	static const uint32_t Purple	= 0xFF00FFFF;
	static const uint32_t Orange	= 0xFFA500FF;
	static const uint32_t Pink		= 0xFF69B4FF;
	static const uint32_t Gold		= 0xFFD700FF;

	Color()
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			u.vec[ i ] = 0.0f;
		}
	}

	Color( uint32_t _color )
	{
		Pixel pixel;
		pixel.r8g8b8a8 = _color;

		for ( int32_t i = 0; i < 4; ++i )
		{
			(*this)[ i ] = pixel[ i ] / 255.0f;
		}
	}

	Color( const float _r, const float _g, const float _b, const float _a )
	{
		u.rgba.r	= std::max( 0.0f, _r );
		u.rgba.g	= std::max( 0.0f, _g );
		u.rgba.b	= std::max( 0.0f, _b );
		u.rgba.a	= _a;
	}

	Color( const float _r, const float _g, const float _b )
	{
		u.rgba.r	= std::max( 0.0f, _r );
		u.rgba.g	= std::max( 0.0f, _g );
		u.rgba.b	= std::max( 0.0f, _b );
		u.rgba.a	= 1.0f;
	}

	Color( const float _value )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			u.vec[ i ] = _value;
		}
	}

	Color( const RgbTuple<float> rgb, float a = 1.0f )
	{
		u.rgba.r = rgb.r;
		u.rgba.g = rgb.g;
		u.rgba.b = rgb.b;
		u.rgba.a = a;
	}

	float& operator[]( const uint32_t i )
	{
		if ( i >= 4 )
		{
			assert( false );
			return u.vec[ 4 ];
		}

		return u.vec[ 3 - i ];
	}

	float operator[]( const uint32_t i ) const
	{
		if ( i >= 4 )
		{
			assert( false );
			return u.vec[ 4 ];
		}

		return u.vec[ 3 - i ];
	}

	Color& operator+=( const Color& color )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			u.vec[ i ] += color.u.vec[ i ];
		}
		return *this;
	}

	Color& operator-=( const Color& color )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			u.vec[ i ] -= color.u.vec[ i ];
		}
		return *this;
	}

	Color& operator*=( const Color& color )
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			u.vec[ i ] *= color.u.vec[ i ];
		}
		return *this;
	}

	bool operator==( const Color& color ) const
	{
		for ( int32_t i = 0; i < 4; ++i )
		{
			if ( fabs( u.vec[ i ] - color.u.vec[ i ] ) > 1e-7 )
			{
				return false;
			}
		}
		return true;
	}

	inline float& r()
	{
		return u.rgba.r;
	}

	inline float& g()
	{
		return u.rgba.g;
	}

	inline float& b()
	{
		return u.rgba.b;
	}

	inline float& a()
	{
		return u.rgba.a;
	}

	inline RgbaTuple<float>& rgba()
	{
		return u.rgba;
	}

	inline float r() const
	{
		return u.rgba.r;
	}

	inline float g() const
	{
		return u.rgba.g;
	}

	inline float b() const
	{
		return u.rgba.b;
	}

	inline float a() const
	{
		return u.rgba.a;
	}

	inline RgbaTuple<float> rgba() const
	{
		return u.rgba;
	}

	inline RgbTuple<float> rgb() const
	{
		RgbTuple<float> rgb;
		rgb.r = u.rgba.r;
		rgb.g = u.rgba.g;
		rgb.b = u.rgba.b;
		return rgb;
	}

	inline Color Inverse() const
	{
		Color outColor;
		for ( int32_t i = 0; i < 4; ++i )
		{
			outColor[ i ] = ( 1.0f - std::min( 1.0f, std::max( 0.0f, u.vec[ i ] ) ) );
		}
		return outColor;
	}

	inline RGBA AsRGBA() const
	{
		RGBA rgba;
		rgba.r = static_cast<uint8_t>( 255.0f * u.rgba.r );
		rgba.g = static_cast<uint8_t>( 255.0f * u.rgba.g );
		rgba.b = static_cast<uint8_t>( 255.0f * u.rgba.b );
		rgba.a = static_cast<uint8_t>( 255.0f * u.rgba.a );
		return rgba;
	}

	inline uint32_t AsR8G8B8A8() const
	{
		const float r = std::min( 1.0f, std::max( 0.0f, u.rgba.r ) );
		const float g = std::min( 1.0f, std::max( 0.0f, u.rgba.g ) );
		const float b = std::min( 1.0f, std::max( 0.0f, u.rgba.b ) );
		const float a = std::min( 1.0f, std::max( 0.0f, u.rgba.a ) );

		const uint8_t p0 = static_cast<uint8_t>( 255.0f * r );
		const uint8_t p1 = static_cast<uint8_t>( 255.0f * g );
		const uint8_t p2 = static_cast<uint8_t>( 255.0f * b );
		const uint8_t p3 = static_cast<uint8_t>( 255.0f * a );

		Pixel abgr;
		abgr.rgba = { p3, p2, p1, p0 };
		return abgr.r8g8b8a8;
	}

private:
	union color_t
	{
		float				vec[ 5 ];
		RgbaTuple<float>	rgba;
	} u;
};


inline Color operator*( const float t, const Color& color )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = t * color[ i ];
	}
	return outColor;
}


inline Color operator*( const Color& color, const float t )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = t * color[ i ];
	}
	return outColor;
}


inline Color operator*( const Color& color0, const Color& color )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = color0[ i ] * color[ i ];
	}
	return outColor;
}


inline Color operator+( const Color& color0, const Color& color1 )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = color0[ i ] + color1[ i ];
	}
	return outColor;
}


inline Color operator-( const Color& color0, const Color& color1 )
{
	Color outColor;
	for ( int32_t i = 0; i < 4; ++i )
	{
		outColor[ i ] = color0[ i ] - color1[ i ];
	}
	return outColor;
}


Color SrgbTolinear( const Color& color, const float gamma = 2.2f );
Color LinearToSrgb( const Color& color, const float gamma = 2.2f );
Color BlendColor( const Color& src, const Color& dest, const blendMode_t blendMode );
