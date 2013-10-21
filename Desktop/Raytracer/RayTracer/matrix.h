#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
#include"MVector.h"

//------------------------------------------------------------------------//
// Matrix Class and matrix functions
//------------------------------------------------------------------------//

template< size_t N, typename T>
class MatrixRow
{
private:
	
	T column_data[N];

public:

	inline T& operator[](size_t i){
		return column_data[i];
	}

	inline const T& operator[](size_t i) const{
		return column_data[i];
	}
};

template< size_t M, size_t N, typename T>
class Matrix {

private:
	MatrixRow<N, T> data[M];

public:	
	
	static const size_t rows = N;
	static const size_t cols = M;

	Matrix();
	Matrix(T[]);

	Matrix<N, M, T>	transpose(void);
	Matrix<N, M, T>	transpose(void) const;
	bool Matrix<M, N, T>::isInvertible();
//	Matrix<M, N, T>	Inverse(bool&);
/*
	Matrix& operator+=(Matrix& m);
	Matrix& operator-=(Matrix& m);
	Matrix& operator*=(T s);
	Matrix& operator/=(T s);
*/
	MatrixRow<N, T>& operator[](const unsigned size_t);
	const MatrixRow<N, T>& operator[](const unsigned size_t) const;
};

template< size_t N, typename T>
class MatrixSubViewRow
{
private:
	
	T column_data[N];

public:

	inline T& operator[](size_t i){
		return column_data[i];
	}

	inline const T& operator[](size_t i) const{
		return column_data[i];
	}
};

template< size_t M, size_t N, typename T>
class MatrixSubView{

private:

	size_t i, j;
	Matrix<M, N, T>& ref_Matrix;

public:

	MatrixSubView(const Matrix< M, N, T>& m, const size_t col_mask, const size_t row_mask): i(col_mask), j(row_mask), ref_Matrix(m){}

	MatrixSubViewRow<N, T>& operator[](size_t i);
	const MatrixSubViewRow<N, T>& operator[](size_t i) const;
};

template< typename T>
T det(Matrix<2, 2, T> m);
template< typename T>
T det(Matrix<3, 3, T> m);
template< typename T>
T det(Matrix<4, 4, T> m);

typedef Matrix<2, 2, double> mat2x2d;
typedef Matrix<2, 2, float> mat2x2f;

typedef Matrix<3, 3, double> mat3x3d;
typedef Matrix<3, 3, float> mat3x3f;

typedef Matrix<4, 4, double> mat4x4d;
typedef Matrix<4, 4, float> mat4x4f;

template< size_t M, size_t N, typename T>
Matrix<M, N, T>::Matrix()
{
	for(size_t j(0); j < rows; ++j){
		for(size_t i(0); i < cols; ++i){
			data[j][i] = static_cast<T>(0.0);
		}
	}
}

template< size_t M, size_t N, typename T>
Matrix<M, N, T>::Matrix(T values[])
{
	for(size_t j(0); j < M; ++j){
		for(size_t i(0); i < N; ++i){
			data[j][i] = values[i+(j*N)];
		}
	}
}

template< size_t M, size_t N, typename T>
Matrix<N, M, T> Matrix<M, N, T>::transpose(void)
{
	Matrix<N, M, T> mt;
	for(size_t c(0); c < N; ++c){
		for(size_t r(0); r < M; ++r)
		{
			mt[c][r] = data[r][c];
		}
	}
	return mt;
}

template< size_t M, size_t N, typename T>
Matrix<N, M, T> Matrix<M, N, T>::transpose(void) const
{
	Matrix<N, M, T> mt;
	for(size_t c(0); c < N; ++c){
		for(size_t r(0); r < M; ++r)
		{
			mt[c][r] = data[r][c];
		}
	}
	return mt;
}

template< size_t M, size_t N, typename T>
bool Matrix<M, N, T>::isInvertible(){
	return (det(*this)!=0);
}
/*
Matrix& Matrix::operator+=(Matrix& m)
{
	*this = *this + m;
	return *this;
}

Matrix& Matrix::operator-=(Matrix& m)
{
	return *this - m;
}

Matrix& Matrix::operator*=(T s)
{
	return *this * s;
}

Matrix& Matrix::operator/=(T s)
{
	return *this / s;
}*/

template< size_t M, size_t N, typename T>
MatrixRow<N, T>& Matrix<M, N, T>::operator[](const unsigned int i) 
{
	return data[i];
}

template< size_t M, size_t N, typename T>
const MatrixRow<N, T>& Matrix<M, N, T>::operator[](const unsigned int i) const
{
	return data[i];
}

template< size_t M, size_t N, typename T>
Matrix<M, N, T> operator+(const Matrix<M, N, T>& m1, const Matrix<M, N, T>& m2)
{
	Matrix<M, N, T> m3;
	for(size_t c(0); c < N; ++c){
		for(size_t r(0); r < M; ++r){
			m3[c][r] = m1[c][r] + m2[c][r];
		}
	}
	return	m3;
}

template< size_t M, size_t N, typename T>
Matrix<M, N, T> operator-(const Matrix<M, N, T>& m1, const Matrix<M, N, T>& m2)
{
	Matrix<M, N, T> m3;
	for(size_t c(0); c < N; ++c){
		for(size_t r(0); r < M; ++r){
			m3[c][r] = m1[c][r] - m2[c][r];
		}
	}
	return	m3;
}

template< size_t M, size_t N, typename T>
Matrix<M, N, T> operator/(const Matrix<M, N, T>& m, T s)
{	
	Matrix<M, N, T> md;
	for(size_t c(0); c < M; ++c){
		for(size_t r(0); r < N; ++r){
			md[c][r] = m[c][r] / s;
		}
	}
	return	md;
}

template< size_t M1, size_t N1, size_t N2, typename T>
Matrix<M1, N2, T> operator*(const Matrix<M1, N1, T>& m1, const Matrix<N1, N2, T>& m2)
{
	Matrix<M1, N2, T> m3;
	for(size_t r(0); r < M1; ++r){
		for(size_t c(0); c < N2; ++c){
			for(size_t a(0); a < M1; ++a){
				m3[r][c] += m1[r][a] * m2[a][c];
			}
		}
	}
	return m3;
}

template< size_t M, size_t N, typename T>
Matrix<M, N, T> operator*(const Matrix<M, N, T>& m, T s)
{
	Matrix<M, N, T> md;
	for(size_t c(0); c < N; ++c){
		for(size_t r(0); r < M; ++r){
			md[c][r] = m[c][r] * s;
		}
	}
	return	md;
}

template< size_t M, size_t N, typename T>
Matrix<M, N, T> operator*(T s, const Matrix<M, N, T>& m)
{
	return m*s;
}

template< size_t M, size_t N, typename T>
MVector<N, T> operator*(const MVector<N, T>& u, const Matrix<M, N, T>& m)
{
	vec4d v;
	for(size_t r(0); r < M; ++r){
		for(size_t c(0), rd(0); c < N; ++c, ++rd){
			v[rd] += u[r] * m[c][r];  
		}
	}	

	return v;
}

template< size_t M, size_t N, typename T>
MVector<N, T> operator*(const Matrix<M, N, T>& m, const MVector<M, T>& u)
{
	return u*m;
}

template< typename T>
T det(Matrix<2, 2, T> m)
{
	return m[0][0]*m[1][1] - m[1][0]*m[0][1];	
}

template< typename T>
T det(Matrix<3, 3, T> m)
{
	double cof00[] = { m[1][1], m[1][2], m[2][1], m[2][2] };
	double cof01[] = { m[1][0], m[1][2], m[2][0], m[2][2] };
	double cof02[] = { m[1][0], m[1][1], m[2][0], m[2][1] };

	return m[0][0]*det(mat2x2d(cof00)) - m[0][1]*det(mat2x2d(cof01)) + m[0][2]*det(mat2x2d(cof02));
}

template< typename T>
T det(Matrix<4, 4, T> m)
{
	double cof00[] = { m[1][1], m[1][2], m[1][3],  m[2][1], m[2][2], m[2][3],  m[3][1], m[3][2], m[3][3]};
	double cof01[] = { m[1][0], m[1][2], m[1][3],  m[2][0], m[2][2], m[2][3],  m[3][0], m[3][2], m[3][3]};
	double cof02[] = { m[1][0], m[1][1], m[1][3],  m[2][0], m[2][1], m[2][3],  m[3][0], m[3][1], m[3][3]};
	double cof03[] = { m[1][0], m[1][1], m[1][2],  m[2][0], m[2][1], m[2][2],  m[3][0], m[3][1], m[3][2]};

	return m[0][0]*det(mat3x3d(cof00)) - m[0][1]*det(mat3x3d(cof01)) + m[0][2]*det(mat3x3d(cof02)) - m[0][3]*det(mat3x3d(cof03));
}

template< size_t M, typename T>
Matrix<M, M, T> inverse(const Matrix<M, M, T>& m, bool& invertible)
{
	T det_val = det(m);

	if(det_val==0){ invertible=false; return Matrix<M, M, T>();}
	invertible=true;

	return m.transpose()*(1./det_val);
}

template< size_t M, size_t N, typename T >
ostream& operator<<(ostream& stream, const Matrix<M, N, T>& m)
{
	for(size_t r(0); r < M; ++r){
		stream << "[";
		for(size_t c(0); c < N; ++c){
			stream << " " << m[r][c];
		}
		stream << "]" << endl;
	}

	return stream;
}

#endif