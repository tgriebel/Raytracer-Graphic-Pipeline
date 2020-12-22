#pragma once

#include <vector>
#include "mathVector.h"
#include "matrix.h"
#include "model.h"
#include "camera.h"

struct light_t
{
	vec3d pos;
	double intensity;
	Color color;
};

class Scene
{
public:
	std::vector<ModelInstance>	models;
	std::vector<light_t>		lights;
	AABB						aabb;	// TODO: Replace with bvh tree
};


class SceneView
{
public:
	Camera	camera;
	mat4x4d	viewTransform;
	mat4x4d	projTransform;
	mat4x4d	projView;
	vec2i	targetSize;
};