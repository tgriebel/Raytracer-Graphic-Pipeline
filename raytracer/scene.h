/*
* MIT License
*
* Copyright( c ) 2023 Thomas Griebel
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this softwareand associated documentation files( the "Software" ), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <vector>
#include <gfxcore/scene/camera.h>
#include <gfxcore/image/color.h>
#include <gfxcore/math/vector.h>
#include <gfxcore/math/matrix.h>
#include <gfxcore/primitives/geom.h>
#include <gfxcore/scene/scene.h>


class RtScene
{
public:
	std::vector<RtModel>	models;
	std::vector<light_t>	lights;
	AABB					aabb;	// TODO: Replace with bvh tree
	const Scene*			scene;
	AssetManager*			assets;
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