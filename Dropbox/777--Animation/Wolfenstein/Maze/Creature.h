#ifndef ___Creature___
#define ___Creature___
#include "animation_class.h"
#include <vector>
//#include "animation.h"
class Creature{
	//will control multiple animation layers and sound played on certain frames
	//enemies/player inherents
private: 
	float x, z;
	float xspd, zspd;
	float target[2];
	bool deadflag;
	bool dieing;
	int health;
	//vector<Animation> layers;//need to be pointers
	Animation& animations;
public:

	Creature(Animation& ani);
	Creature& operator=(const Creature&);

	inline float getX()	{return x;}
	inline float getZ()	{return z;}

	void setStart(float xi, float zi);
	void setTarget(float xi, float zi);

	void brain();
	void moveLeft();
	void moveRight();
	void moveForward();
	void moveBackward();
	void die();
	bool isDead();
	void draw();


};


#endif