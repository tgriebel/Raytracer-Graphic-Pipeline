#ifndef __CAMERA3D_FREE__
#define __CAMERA3D_FREE__

#include<cmath>
#include<iostream>
using namespace std;
#define PI 4*atan(1.0)

#include "quaternion.h"
#include "MVector.h"

class FreeCamera{

protected:
	float focal;

	float speed, rot_speed;
	float hDegree, vDegree;

	//TO DO: should be utilities!
	inline void normalizeDegree(float &degree){ if(degree>=360) degree = 0;}
	inline float degreeToRadian(float degree) { return degree*(PI/180.0);}
	inline float radianToDegree(float radian) { return radian*(180/PI);}
	
public:

	float fov;
	vec3f at;
	vec3f up;
	vec3f pos;
	vec3f side;

	FreeCamera()
	: 
		at(0.0f, 0.0f, 1.0f), 
		up(0.0f, 1.0f, 0.0f), 
		pos(0.0f, 0.0f, 0.0f)
	{ 
		speed = .5;
		rot_speed = 1;
		hDegree = 90; vDegree = 0;
		fov = 78;

		side = (at - pos)^up;
	}
	FreeCamera(float spdi, float r_spdi, float posx, float posy, float posz, float tilt, float rot)
	:
		at(posx, posy, posz - 1.0f), 
		up(0.0f, 1.0f, 0.0f), 
		pos(posx, posy, posz)
	{
		speed = spdi;
		rot_speed = r_spdi;
		fov = 78;

		hDegree = rot; vDegree = tilt;

		side = (at - pos)^up;
	}

	virtual ~FreeCamera(){}

	void dollyForward();//x
	void dollyBack();//x
	void dollyTargetForward();//x
	void dollyTargetBack();//x
	void dollyCameraForward();//x
	void dollyCameraBack();//x
	void virtual truckUp();//x
	void virtual truckDown();//x
	void virtual truckLeft();//x
	void virtual truckRight();//x
	void virtual rotate(float theta, vec3f& axis, vec3f& point);

	inline void setRotSpeed(float rot_speed_s)  { rot_speed = rot_speed_s;}
	inline void setSpeed(float speed_s)         { speed	    = speed_s;    }

	inline float getRotSpeed(){ return rot_speed;}
	inline float getSpeed()   { return speed;    }
	inline int	 getHDegree() { return hDegree;  }

	void virtual panX(float theta);//x
	void virtual panY(float theta);//x

	void virtual orbitX(float theta);//x
	void virtual orbitY(float theta);//x

	void virtual roll(float theta);//x
};

#endif