#include "quaternion.h"

double length(quaternion quat) {  
	return sqrt(quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w); 
} 

quaternion normalize(quaternion quat) {   
	double L = length(quat);   
	quat.x /= L;   
	quat.y /= L;   
	quat.z /= L;   
	quat.w /= L;
	return quat;
} 

quaternion conjugate(quaternion quat) {
	quat.x = -quat.x;   
	quat.y = -quat.y;   
	quat.z = -quat.z;
	return quat;
}

quaternion mult(quaternion A, quaternion B) {
	quaternion C;  
	C.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;   
	C.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;   
	C.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;   
	C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;   
	return C;
}