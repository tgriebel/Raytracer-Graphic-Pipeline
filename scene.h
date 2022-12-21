#pragma once

#include <vector>
#include "../GfxCore/mathVector.h"
#include "../GfxCore/matrix.h"
#include "../GfxCore/camera.h"
#include "../GfxCore/color.h"
#include "../GfxCore/geom.h"

struct light_t
{
	vec3f	pos;
	vec3f	intensity;
	Color	color;
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
	Camera		camera;
	mat4x4f		viewTransform;
	mat4x4f		projTransform;
	mat4x4f		projView;
	vec2i		targetSize;
	blendMode_t	blendMode;
};