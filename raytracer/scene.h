#pragma once

#include <vector>
#include <scene/camera.h>
#include <image/color.h>
#include <math/vector.h>
#include <math/matrix.h>
#include <primitives/geom.h>
#include <scene/scene.h>


class RtScene
{
public:
	std::vector<RtModel>	models;
	std::vector<light_t>	lights;
	AABB					aabb;	// TODO: Replace with bvh tree
	Scene*					scene;
};


class RtView
{
public:
	Camera		camera;
	mat4x4f		viewTransform;
	mat4x4f		projTransform;
	mat4x4f		projView;
	vec2i		targetSize;
	blendMode_t	blendMode;
};