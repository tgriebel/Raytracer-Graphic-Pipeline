#include "FreeCamera.h"

void FreeCamera::dollyForward(){

	vec3f dv = (at - pos)*speed;
	pos += dv;
	at += dv;
}

void FreeCamera::dollyBack(){

	vec3f dv = (at - pos)*speed;
	pos -= dv;
	at -= dv;
}

void FreeCamera::dollyCameraForward(){

	vec3f dv = (at - pos)*speed;
	pos += dv;
}

void FreeCamera::dollyCameraBack(){

	vec3f dv = (at - pos)*speed;
	pos-= dv;
}

void FreeCamera::dollyTargetForward(){

	vec3f dv = (at - pos)*speed;
	at += dv;
}

void FreeCamera::dollyTargetBack(){

	vec3f dv = (at - pos)*speed;
	at -= dv;
}

void FreeCamera::truckUp(){ 
	
	vec3f dv = up*speed;
	pos += dv;
	at += dv;
}
void FreeCamera::truckDown(){

	vec3f dv = up*speed;
	pos -= dv;
	at -= dv;
}
void FreeCamera::truckLeft(){

	side = (at - pos).normalize()^up;

	vec3f dv = side*speed;
	pos -= dv;
	at -= dv;
}
void FreeCamera::truckRight(){

	side = (at - pos).normalize()^up;

	vec3f dv = side*speed;
	pos += dv;
	at += dv;
}

void FreeCamera::orbitX(float theta){

	float phi = degreeToRadian(theta);

	vec3f v =  pos - at;

	rotate(phi, up, v);

	pos = at + v;
}

void FreeCamera::orbitY(float theta){

	float phi = degreeToRadian(theta);

	vec3f p = pos - at;

	vec3f v = p.normalize()^up;

	rotate(phi, v, p);
	pos = at + p;

	rotate(phi, v, up);
}

void FreeCamera::panX(float theta){

	float phi = degreeToRadian(theta);

	vec3f v = at - pos;

	rotate(phi, up, v);

	at = v + pos;
}

void FreeCamera::panY(float theta){

	float phi = degreeToRadian(theta);

	vec3f p = at - pos;

	vec3f v = p.normalize()^up;

	rotate(phi, v, p);
	at = p + pos;

	rotate(phi, v, up);
}

void FreeCamera::roll(float theta){

	float phi = degreeToRadian(theta);

	vec3f v = at - pos;

	rotate(phi, v, up);
}

void FreeCamera::rotate(float theta, vec3f& axis, vec3f& point){

		axis = axis.normalize();

		quaternion V, R, Rp, U;

		V.x = point[0];
		V.y = point[1];
		V.z = point[2];
		V.w = 0.0;

		R.x = axis[0] * sin(theta/2.0);
		R.y = axis[1] * sin(theta/2.0);
		R.z = axis[2] * sin(theta/2.0);
		R.w = cos(theta/2.0);

		Rp = conjugate(R);
		V = mult(mult(R, V), Rp);

		point[0] = V.x;
		point[1] = V.y;
		point[2] = V.z;
}
