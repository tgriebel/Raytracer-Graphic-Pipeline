#ifndef ___QUATERNION___
#define ___QUATERNION___

#pragma once

#include<cmath>

struct quaternion{
	double x, y, z, w;
};

struct quaternion;

double length(quaternion quat);

quaternion normalize(quaternion quat);

quaternion conjugate(quaternion quat);

quaternion mult(quaternion A, quaternion B);

#endif