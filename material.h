#pragma once

#include <cinttypes>
#include "mathVector.h"
#include "matrix.h"

struct lightInput_t
{
	vec3d	viewVector;
	vec3d	normal;
};

struct material_t
{
	double		Ka;
	double		Ke;
	double		Kd;
	double		Ks;
	double		Tf;
	double		Tr;
	double		Ns;
	double		Ni;
	double		d;
	double		illum;

	bool		textured;
	int32_t		colorMapId;
	int32_t		normalMapId;
};