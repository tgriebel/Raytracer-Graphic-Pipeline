//////////////////////////////////////////////////////////
//														//
// Copyright: Thomas Griebel 2013						//
// Date Modified: 12/02/2020							//
// Name: Vector											//
//														//
// Description: A general solution for a math vector	//
//														//
//////////////////////////////////////////////////////////

#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <iostream>
#include <string>
#include <assert.h>

static double PI = 3.14159265358979323846;

template <size_t D, typename T>
class Vector
{
private:
	T data[ D + 1 ];
	static constexpr T epsilon = std::numeric_limits< T >::epsilon() * ( (T) 2.0 );
public:

	static const size_t size = D;

	Vector() { Zero(); };
	Vector( const Vector< D, T>& vec );
	Vector( const Vector< D-1, T>& vec, T value );
	Vector( const T& d1 );
	Vector( const T& d1, const T& d2 );
	Vector( const T& d1, const T& d2, const T& d3 );
	Vector( const T& d1, const T& d2, const T& d3, const T& d4 );
	Vector( T values[] );

	void FlushDenorms();

	T Length() const;
	Vector< D, T> Normalize() const;
	Vector< D, T> Reverse() const;
	void Zero();

	const T& operator []( const size_t i ) const;
	T& operator []( const size_t i );
	Vector<D, T>& operator=( const Vector<D, T>& u );
	Vector<D, T>& operator+=( const Vector<D, T> & u );
	Vector<D, T>& operator-=( const Vector<D, T>& u );
	Vector<D, T>& operator*=( T& s );
	Vector<D, T>& operator/=( T& s );
};

using vec2i = Vector<2, int32_t>;
using vec3i = Vector<3, int32_t>;
using vec4i = Vector<4, int32_t>;
using vec2f = Vector<2, float>;
using vec3f = Vector<3, float>;
using vec4f = Vector<4, float>;
using vec2d = Vector<2, double>;
using vec3d = Vector<3, double>;
using vec4d = Vector<4, double>;

template<size_t D, typename T>
Vector<D, T>::Vector( const Vector<D, T>& vec )
{
	for ( size_t i = 0; i < D; ++i )
	{
		data[ i ] = vec.data[ i ];
	}
	data[ D ] = static_cast<T>( 0.0 );
}


template<size_t D, typename T>
Vector<D, T>::Vector( const Vector< ( D - 1 ), T>& vec, T value )
{
	for ( size_t i = 0; i < ( D - 1 ); ++i )
	{
		data[ i ] = vec[ i ];
	}

	data[ D - 1 ] = value;
	data[ D ] = static_cast<T>( 0.0 );
}


template< size_t D, typename T >
Vector<D, T>::Vector( const T& value )
{
	for ( size_t i = 0; i < D; ++i )
	{
		data[ i ] = value;
	}
	data[ D ] = static_cast<T>( 0.0 );
}


template< size_t D, typename T >
Vector<D, T>::Vector( const T& d1, const T& d2 )
{
	Zero();
	if ( D >= 2 )
	{
		data[ 0 ] = d1;
		data[ 1 ] = d2;
	}
	data[ D ] = static_cast<T>( 0.0 );
}


template <size_t D, typename T>
Vector<D, T>::Vector( const T& d1, const T& d2, const T& d3 )
{
	Zero();
	if ( D >= 3 )
	{
		data[ 0 ] = d1;
		data[ 1 ] = d2;
		data[ 2 ] = d3;
	}
	data[ D ] = static_cast<T>( 0.0 );
}


template<size_t D, typename T>
Vector<D, T>::Vector( const T& d1, const T& d2, const T& d3, const T& d4 )
{
	Zero();
	if ( D >= 4 )
	{
		data[ 0 ] = d1;
		data[ 1 ] = d2;
		data[ 2 ] = d3;
		data[ 3 ] = d4;
	}
	data[ D ] = static_cast<T>( 0.0 );
}


template<size_t D, typename T>
Vector<D, T>::Vector( T values[] )
{
	for ( size_t i = 0; i < D; ++i )
	{
		data[ i ] = values[ i ];
	}
	data[ D ] = static_cast<T>( 0.0 );
}


template<size_t D, typename T>
void Vector<D, T>::FlushDenorms()
{
	for ( size_t i = 0; i < D; ++i )
	{
		const int code = std::fpclassify( data[ i ] );
		if ( ( code == FP_SUBNORMAL ) || ( code == FP_NAN ) )
		{
			data[ i ] = static_cast<T>( 0.0 );
		}
	}
}


template<size_t D, typename T>
T Vector<D, T>::Length() const
{
	T mag = 0.0;
	for ( size_t i = 0; i < D; ++i )
	{
		mag += data[ i ] * data[ i ];
	}

	return sqrt( mag );
}


template<size_t D, typename T>
Vector<D, T> Vector<D, T>::Normalize() const
{
	Vector< D, T> v;

	T m = Length();

	if ( m <= epsilon )
	{
		m = 1.0;
	}

	for ( size_t i = 0; i < D; ++i )
	{
		v.data[ i ] = data[ i ] / m;
		if ( fabs( v.data[ i ] ) <= v.epsilon )
		{
			v.data[ i ] = 0.0;
		}
	}

	return v;
}


template<size_t D, typename T>
Vector<D, T> Vector<D, T>::Reverse() const
{
	Vector< D, T> v;

	for ( size_t i = 0; i < D; ++i )
	{
		v.data[ i ] = -data[ i ];
	}
	
	return v;
}


template<size_t D, typename T>
void Vector<D, T>::Zero()
{
	for ( size_t i = 0; i < ( D + 1 ); ++i )
	{
		data[ i ] = static_cast<T>( 0.0 );
	}
}


template<size_t D, typename T>
const T& Vector<D, T>::operator[]( const size_t i ) const
{
	if ( i >= D )
	{
		assert( false );
		return data[ D ];
	}

	return data[ i ];
}


template<size_t D, typename T>
T& Vector<D, T>::operator[]( const size_t i )
{
	if ( i >= D )
	{
		assert( false );
		return data[ D ];
	}

	return data[ i ];
}


template<size_t D, typename T>
Vector<D, T>& Vector< D, T >::operator=( const Vector<D, T>& vec )
{
	for ( size_t i = 0; i < D; ++i )
	{
		data[ i ] = vec.data[ i ];
	}

	return *this;
}


template<size_t D, typename T>
Vector<D, T>& Vector<D, T>::operator+=( const Vector<D, T>& u )
{
	for ( size_t i = 0; i < D; ++i )
	{
		data[ i ] += u.data[ i ];
	}

	return *this;
}


template<size_t D, typename T>
Vector<D, T>& Vector<D, T>::operator-=( const Vector<D, T>& u )
{
	for ( size_t i = 0; i < D; ++i )
	{
		data[ i ] -= u.data[ i ];
	}

	return *this;
}


template<size_t D, typename T>
Vector<D, T>& Vector< D, T >::operator*=( T& s )
{
	for ( size_t i = 0; i < D; ++i )
	{
		data[ i ] *= s;
	}

	return *this;
}


template<size_t D, typename T>
Vector<D, T>& Vector<D, T>::operator/=( T& s )
{
	for ( size_t i = 0; i < D; ++i )
	{
		data[ i ] /= s;
	}

	return *this;
}


template<size_t D, typename T>
bool operator==( const Vector<D, T>& u, const Vector<D, T>& v )
{
	for ( size_t i = 0; i < D; ++i )
	{
		if ( u[ i ] != v[ i ] )
		{
			return false;
		}
	}

	return true;
}


template<size_t D, typename T>
bool operator !=( const Vector<D, T>& u, const Vector<D, T>& v )
{
	return !( u == v );
}


template<size_t D, typename T>
Vector<D, T> operator+( const Vector<D, T>& u, const Vector<D, T>& v )
{
	Vector<D, T> w;
	for ( size_t i = 0; i < D; ++i )
	{
		w[ i ] = u[ i ] + v[ i ];
	}

	return w;
}


template<size_t D, typename T>
Vector<D, T> operator-( const Vector<D, T>& u, const Vector<D, T>& v )
{
	Vector<D, T> w;
	for ( size_t i = 0; i < D; ++i )
	{
		w[ i ] = u[ i ] - v[ i ];
	}

	return w;
}


template<size_t D, typename T>
T Dot( const Vector<D, T>& u, const Vector<D, T>& v )
{
	T dot( 0.0 );
	for ( size_t i = 0; i < D; ++i )
	{
		dot += u[ i ] * v[ i ];
	}

	return dot;
}


template<size_t D, typename T>
Vector<D, T> operator*( T s, const Vector<D, T>& u )
{
	Vector< D, T> v;
	for ( size_t i = 0; i < D; ++i )
	{
		v[ i ] = u[ i ] * s;
	}

	return v;
}


template< size_t D, typename T >
Vector<D, T> operator*( const Vector<D, T>& u, T s )
{
	return s * u;
}


template< size_t D, typename T >
Vector<D, T> operator/( const Vector<D, T>& u, T s )
{
	Vector<D, T> v;
	for ( size_t i = 0; i < D; ++i )
	{
		v[ i ] = u[ i ] / s;
	}

	return v;
}


template<typename T>
Vector<3, T> Cross( const Vector<3, T>& u, const Vector<3, T>& v )
{
	Vector<3, T> w;
	w[ 0 ] = u[ 1 ] * v[ 2 ] - u[ 2 ] * v[ 1 ];
	w[ 1 ] = u[ 2 ] * v[ 0 ] - u[ 0 ] * v[ 2 ];
	w[ 2 ] = u[ 0 ] * v[ 1 ] - u[ 1 ] * v[ 0 ];

	return w;
}


template< size_t D, typename T>
T Angle( const Vector<D, T>& u, const Vector<D, T>& v )
{
	T theta = static_cast<T>( 0.0 );
	T mag = u.Length() * v.Length();

	if ( mag > u.epsilon )
	{
		T result = Dot( u, v ) / mag;

		if ( result >= ( (T) -1.0 - u.epsilon ) && result <= ( (T) 1.0 + u.epsilon ) )
		{
			theta = acos( result ) * ( (T) 180.0 / (T) PI );
		}
	}
	return theta;
}


template<typename T>
T TripleScalar( const Vector<3, T>& u, const Vector<3, T>& v, const Vector<3, T>& w )
{
	return Dot( Cross( u, v ), w );
}


template<size_t D, typename T>
Vector<D, T> Divide( const Vector<D, T>& u, const Vector<D, T>& v )
{
	Vector<D, T> w;

	for ( size_t i = 0; i < D; ++i )
	{
		w[ i ] = u[ i ] / v[ i ];
	}

	return w;
}


template<size_t D, typename T>
Vector<D, T> Multiply( const Vector<D, T>& u, const Vector<D, T>& v )
{
	Vector<D, T> w;

	for ( size_t i = 0; i < D; ++i )
	{
		w[ i ] = u[ i ] * v[ i ];
	}

	return w;
}


template<size_t SrcLength, size_t TruncNum, typename T, size_t DestLength = ( SrcLength - TruncNum )>
Vector<DestLength, T> Trunc( const Vector< SrcLength, T>& u )
{
	Vector<DestLength, T> dstVec;

	for( size_t i = 0; i < DestLength; ++i )
	{
		dstVec[ i ] = u[ i ];
	}

	return dstVec;
}


template<size_t SrcLength, size_t ConcatNum, typename T, size_t DestLength = ( SrcLength + ConcatNum )>
Vector<DestLength, T> Concat( const Vector<SrcLength, T>& u, const T fillValue = static_cast<T>( 0.0 ) )
{
	Vector<DestLength, T> dstVec;

	for ( size_t i = 0; i < SrcLength; ++i )
	{
		dstVec[ i ] = u[ i ];
	}

	for ( size_t i = SrcLength; i < DestLength; ++i )
	{
		dstVec[ i ] = fillValue;
	}

	return dstVec;
}


template<size_t D, typename T >
std::ostream& operator<<( std::ostream& stream, const Vector< D, T>& v )
{
	stream << "[";
	for ( size_t i = 0; i < D; ++i )
	{
		stream << " " << v[ i ];
	}
	stream << " ]";

	return stream;
}