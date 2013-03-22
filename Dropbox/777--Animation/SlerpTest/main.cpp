

#include <iostream>
#include "..\MocapViewer\Quaternion.h"
#include "..\MocapViewer\MVector.h"
using namespace std;


int main(){

	vec3f v1(1.0,1.0,1.0), v2(1.0,1.0,1.0);

	Quaternion< float > q1( 179.0, v1), q2(90.0, v2);

	cout << "q1: " << q1 << endl;
	cout << "q2: " << q2 << endl;

	float s = 0.0;

	float p(0), h(0), b(0);

	for( int i(0); i <= 8; ++i){
		Quaternion<float> q3 = slerp(q1, q2, s);
		s += 0.125;

		//cout << "S=" << s << " " << q3 << endl;

		p = -2.0*( q3.y*q3.z - q3.w*q3.x);

		if( fabs(p) > 5e-4){

			p = 1.570796f*p;
			h = atan2( (double)(-(q3.x*q3.z) + (q3.w*q3.y) ), (double)(.5 - (q3.y*q3.y)- (q3.z*q3.z)) );
			b = 0.0f;
		}else{
		
			p = asin(p);
			h = atan2( (double)( (q3.x*q3.z) + (q3.w*q3.y) ), (double)(.5 - (q3.x*q3.x) - (q3.y*q3.y) ) );
			b = atan2( (double)( (q3.x*q3.y) + (q3.w*q3.z) ), (double)(.5 - (q3.x*q3.x) - (q3.z*q3.z) ) );
		}
		
		const double pi = 3.14159;
		p = p*(180.0/pi);
		h = h*(180.0/pi);
		b = b*(180.0/pi);

		cout << endl << "p: " << p << " h: " << h << " b: " << b << endl;
		
	}

	char c;
	cin >> c;

	return 1;
}