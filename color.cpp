#include "color.h"

Color SrgbTolinear( const Color& color, const float gamma )
{
	Color outColor;
	for ( int32_t i = 0; i < 3; ++i )
	{
		outColor[ i ] = pow( color[ i ], gamma );
	}

	return outColor;
}


Color LinearToSrgb( const Color& color, const float gamma )
{
	Color outColor;
	for ( int32_t i = 0; i < 3; ++i )
	{
		outColor[ i ] = pow( color[ i ], 1.0f / gamma );
	}

	return outColor;
}


Color BlendColor( const Color& src, const Color& dest, const blendMode_t blendMode )
{
	switch ( blendMode )
	{
		default:
		case blendMode_t::SRCCOLOR:
		{
			return src;
		}
		case blendMode_t::DESTCOLOR:
		{
			return dest;
		}
		case blendMode_t::SRCALPHA:
		{
			return Lerp( dest, src, src.rgba().a );
		}
		case blendMode_t::DESTALPHA:
		{
			return Lerp( dest, src, dest.rgba().a );
		}
		case blendMode_t::INVSRCALPHA:
		{
			return Lerp( dest, src, 1.0f - src.rgba().a );
		}
		case blendMode_t::INVDESTALPHA:
		{
			return Lerp( dest, src, 1.0f - dest.rgba().a );
		}
		case blendMode_t::INVSRCCOLOR:
		{
			return src.Inverse();
		}
		case blendMode_t::INVDESTCOLOR:
		{
			return dest.Inverse();
		}
		case blendMode_t::ADD:
		{
			return ( src + dest );
		}
		case blendMode_t::SUBTRACT:
		{
			return ( src - dest );
		}
		case blendMode_t::REVSUBTRACT:
		{
			return ( dest - src );
		}
		case blendMode_t::MIN:
		{
			Color out;
			for ( int32_t i = 0; i < 4; ++i )
			{
				out[ i ] = std::min( src[ i ], dest[ i ] );
			}
			return out;
		}
		case blendMode_t::XOR:
		{
			return Color( src.AsR8G8B8A8() ^ dest.AsR8G8B8A8() );
		}
		case blendMode_t::MAX:
		{
			Color out;
			for ( int32_t i = 0; i < 4; ++i )
			{
				out[ i ] = std::max( src[ i ], dest[ i ] );
			}
			return out;
		}
		case blendMode_t::ZERO:
		{
			return Color( 0.0f );
		}
		case blendMode_t::ONE:
		{
			return Color( 1.0f );
		}
	}
}
