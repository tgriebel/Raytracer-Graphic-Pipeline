//////////////////////////////////////////////////////////
//														//
// Copyright: Thomas Griebel 2013						//
// Date Modified: 3/02/2013								//
// Name: MVector										//
//														//
// Description: A general solution for a math Vector	//
//														//
//////////////////////////////////////////////////////////

#ifndef __MATHVector__
#define __MATHVector__

#include <math.h>
#include <limits>
#include <iostream>
#include <string>
using namespace std;

template <size_t D, typename T>

class MVector;

template <size_t D, typename T>
bool operator==(const MVector< D, T>& u, const MVector< D, T>& v);

template <size_t D, typename T>
bool operator!=(const MVector< D, T>& u, const MVector< D, T>& v);

template <size_t D, typename T>
MVector< D, T> operator+(const MVector< D, T>& u, const MVector< D, T>& v);

template <size_t D, typename T>
MVector< D, T> operator-(const MVector< D, T>& u, const MVector< D, T>& v);

template <size_t D, typename T>
T operator*(const MVector< D, T>& u, const MVector< D, T>& v);

template <size_t D, typename T>
T operator*(const MVector< D, T>& u, const MVector< D, T>& v);

template <size_t D, typename T>
MVector< D, T> operator*(T s, const MVector< D, T>& u);

template <size_t D, typename T>
MVector< D, T> operator*(const MVector< D, T>& u, T s);

template < size_t D, typename T>
MVector< D, T> operator/(const MVector< D, T>& u, T s);

template <typename T>
MVector< 3, T> operator^(const MVector< 3, T>& u, const MVector< 3, T>& v);

template <size_t D, typename T>
T angle(const MVector< D, T>& u, const MVector< D, T>& v);

template <typename T>
T tripleScalarProduct(const MVector< 3, T>& u, const MVector< 3, T>& v, const MVector< 3, T>& w);

template <size_t D, typename T>
ostream & operator<< (ostream &, const MVector< D, T>&);

template <size_t D, typename T>
class MVector {

private:
	T data[D];
	T tol;
public:

	static const size_t size = D;

	//Constructors
	MVector(): tol(0.0) { zero();};
	MVector(const MVector< D, T>& vec);
	MVector(const T& d1);
	MVector(const T& d1, const T& d2);
	MVector(const T& d1, const T& d2, const T& d3);
	MVector(const T& d1, const T& d2, const T& d3, const T& d4);
	MVector(T values[]);

	T magnitude() const;
	MVector< D, T> normalize() const;
	void reverse();
	void zero();

	const T& operator [](const size_t i) const;
	T& operator [](const size_t i);
	MVector< D, T >& operator =(const MVector& u);
	MVector< D, T >& operator +=(const MVector& u);
	MVector< D, T >& operator -=(const MVector& u);
	MVector< D, T >& operator *=(T s);
	MVector< D, T >& operator /=(T s);
	
	friend bool operator==< D, T>(const MVector< D, T>& u, const MVector< D, T>& v);
	friend bool operator!=< D, T>(const MVector< D, T>& u, const MVector< D, T>& v);
	friend MVector< D, T> operator+< D, T>(const MVector< D, T>& u, const MVector< D, T>& v);
	friend MVector< D, T> operator-< D, T>(const MVector< D, T>& u, const MVector< D, T>& v);
	friend T operator*< D, T>(const MVector< D, T>& u, const MVector< D, T>& v);
	friend MVector< D, T> operator*< D, T>(T s, const MVector< D, T>& u);
	friend MVector< D, T> operator*< D, T >(const MVector< D, T>& u, T s);
	friend MVector< D, T> operator/< D, T>(const MVector< D, T>& u, T s);
	friend MVector< 3, T> operator^< T>(const MVector< 3, T>& u, const MVector< 3, T>& v);
	friend T angle< D, T>( const MVector< D, T>& u, const MVector< D, T>& v );
	friend T tripleScalarProduct< T>(const MVector< 3, T>& u, const MVector< 3, T>& v, const MVector< 3, T>& w);
	friend ostream & operator<< < D, T>(ostream &, const MVector< D, T>&);
};

typedef MVector< 2, double> vec2f;
typedef MVector< 3, double> vec3f;
typedef MVector< 4, double> vec4f;
typedef MVector< 2, double> vec2d;
typedef MVector< 3, double> vec3d;
typedef MVector< 4, double> vec4d;

template< size_t D, typename T >
MVector< D, T >::MVector(const MVector< D, T>& vec)
{		
	tol = vec.tol;
	for(size_t i(0); i < D; ++i)
		data[i] = vec.data[i];
}

template< size_t D, typename T >
MVector< D, T >::MVector(const T& d1)
{
	tol = std::numeric_limits< T >::epsilon()*((T)2.0);
	data[0] = d1;
}

template< size_t D, typename T >
MVector< D, T >::MVector(const T& d1, const T& d2)
{
	tol = std::numeric_limits< T >::epsilon()*((T)2.0);
	data[0] = d1;	data[1] = d2;
}

template <size_t D, typename T>
MVector< D, T >::MVector(const T& d1, const T& d2, const T& d3)
{
	tol = std::numeric_limits< T >::epsilon()*((T)2.0);
	data[0] = d1;	data[1] = d2;	data[2] = d3;
}

template< size_t D, typename T >
MVector< D, T >::MVector(const T& d1, const T& d2, const T& d3, const T& d4)
{
	tol = std::numeric_limits< T >::epsilon()*((T)*2.0);
	data[0] = d1;	data[1] = d2;	data[2] = d3;	data[3] = d4;
}
template< size_t D, typename T >
MVector< D, T>::MVector(T values[])
{
	tol = std::numeric_limits< T >::epsilon()*((T)*2.0);
	for(size_t i(0); i < D; ++i)
		data[i] = values[i];
}

template< size_t D, typename T >
T MVector< D, T >::magnitude() const
{
	T mag = 0.0;
	for(size_t i(0); i < D; ++i)
		mag += data[i] * data[i];

	return sqrt(mag);
}

template< size_t D, typename T >
MVector< D, T> MVector< D, T >::normalize() const
{
	MVector< D, T> v = *this;
	
	T m = v.magnitude();

	if(m <= v.tol) m = 1.0;

	for(size_t i(0); i < D; ++i){
		v.data[i] /= m;
		if(fabs(v.data[i]) <= v.tol) v.data[i] = 0.0;
	}

	return v;
}

template< size_t D, typename T >
void MVector< D, T >::reverse()
{
	for(size_t i(0); i < D; ++i)
		data[i] = -data[i];
}

template< size_t D, typename T >
void MVector< D, T >::zero()
{
	for(size_t i(0); i < D; ++i)
		data[i] = 0.0;
}

template< size_t D, typename T >
const T& MVector< D, T >::operator [](const size_t i) const
{
	return data[i];
}

template< size_t D, typename T >
T& MVector< D, T >::operator [](const size_t i)
{
	return data[i];
}

template< size_t D, typename T >
MVector< D, T>& MVector< D, T >::operator =(const MVector< D, T>& vec)
{		
	tol = vec.tol;
	for(size_t i(0); i < D; ++i)
		data[i] = vec.data[i];

	return *this;
}

template< size_t D, typename T >
MVector< D, T >& MVector< D, T >::operator +=(const MVector& u)
{
	for(size_t i(0); i < D; ++i)
		data[i] += u.data[i];
	
	return *this;
}

template< size_t D, typename T >
MVector< D, T >& MVector< D, T >::operator -=(const MVector& u)
{
	for(size_t i(0); i < D; ++i)
		data[i] -= u.data[i];

	return *this;
}

template< size_t D, typename T >
MVector< D, T >& MVector< D, T >::operator *=(T s)
{
	for(size_t i(0); i < D; ++i)
		data[i] *= s;
	
	return *this;
}

template< size_t D, typename T >
MVector< D, T >& MVector< D, T >::operator /=(T s)
{
	for(size_t i(0); i < D; ++i)
		data[i] /= s;
	
	return *this;
}

template< size_t D, typename T >
bool operator==(const MVector< D, T>& u, const MVector< D, T>& v)
{
	for(size_t i(0); i < D; ++i)
		if(u.data[i] != v.data[i])
			return false;

	return true;
}

template< size_t D, typename T >
bool operator !=(const MVector< D, T>& u, const MVector< D, T>& v)
{
	return !(u ==v);
}

template< size_t D, typename T >
MVector< D, T > operator+(const MVector< D, T>& u, const MVector< D, T>& v)
{
	MVector< D, T > w;
	for(size_t i(0); i < D; ++i)
		w.data[i] = u.data[i] + v.data[i];
	
	return w;
}

template< size_t D, typename T >
MVector< D, T > operator-(const MVector< D, T>& u, const MVector< D, T>& v)
{
	MVector< D, T> w;
	for(size_t i(0); i < D; ++i)
		w.data[i] = u.data[i] - v.data[i];
	
	return w;
}

//MVector dot product
template< size_t D, typename T >
T operator*(const MVector< D, T>& u, const MVector< D, T>& v)
{
	T dot(0.0);
	for(size_t i(0); i < D; ++i)
		dot += u.data[i]*v.data[i];

	return dot;
}

//scalar multiplication
template< size_t D, typename T >
MVector< D, T > operator*(T s, const MVector< D, T>& u)
{
	MVector< D, T> v;
	for(size_t i(0); i < D; ++i)
		v.data[i] = u.data[i] * s;

	return v;
}

template< size_t D, typename T >
MVector< D, T> operator*(const MVector< D, T>& u, T s)
{
	return s*u;
}

template< size_t D, typename T >
MVector< D, T> operator/(const MVector< D, T>& u, T s){

	MVector< D, T> v;
	for(size_t i(0); i < D; ++i)
		v.data[i] = u.data[i] / s;
	
	return v;
}

template<typename T>
MVector<3, T> operator^(const MVector<3, T>& u, const MVector<3, T>& v){
	MVector<3, T> w;

	w.data[0] = u.data[1]*v.data[2] - u.data[2]*v.data[1];
	w.data[1] = u.data[2]*v.data[0] - u.data[0]*v.data[2];
	w.data[2] = u.data[0]*v.data[1] - u.data[1]*v.data[0];

	return w;
}

template< size_t D, typename T>
T angle( const MVector<D, T>& u, const MVector<D, T>& v ){
	
	T theta((T)0.0);
	T mag = u.magnitude()*v.magnitude();

	if( mag > u.tol ){

		T result = (u*v) / mag;
		
		if( result >= ((T)-1.0 - u.tol) && result <= ((T)1.0 + u.tol) ){
			theta = acos( result )*((T)180.0/(T)PI);
		}
	}
	return theta;
}

template<typename T>
T TripleScalarProduct(const MVector< 3, T>& u, const MVector< 3, T>& v, const MVector< 3, T>& w){

	return (u.data[0] * (v.data[1]*w.data[2] - v.data[2]*w.data[1])) +
			(u.data[1] * (-v.data[0]*w.data[2] + v.data[2]*w.data[0]))+
			(u.data[2] * (v.data[0]*w.data[1] - v.data[1]*w.data[0]));
}

template< size_t D, typename T >
ostream& operator<< < D, T>(ostream& stream, const MVector< D, T>& v)
{
	stream << "[";
	for(size_t i = 0; i < D; ++i)
		stream << " " << v.data[i];
	stream << " ]";

	return stream;
}

#endif