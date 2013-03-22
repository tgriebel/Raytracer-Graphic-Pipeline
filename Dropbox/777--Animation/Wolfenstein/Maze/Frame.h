//////////////////////////////////////////////////////////////////////////////
//Name: Frame Class															//
//Description: Abstract Class. Child can specify the contents of the frame	//
//				A frame child might be a 3D or 2D figure					//
//Author: Thomas Griebel													//
//Date Last Modified: 12/23/2010											//
//Date Started: 12/23/2010													//
//////////////////////////////////////////////////////////////////////////////

#ifndef ___FRAME___
#define ___FRAME___

class Frame{

private: 
	float x;
	float y;
	float z;

	float width;
	float length;
	float height;
public:
	Frame():x(0.0),y(0.0),z(0.0), width(1.0), length(1.0), height(1.0){}
	virtual ~Frame(){};

	virtual void draw() = 0;

	inline float getX()						{ return x;}
	inline float getY()						{ return y;}
	inline float getZ()						{ return z;}

	inline void setX(float setX)			{ x = setX;}
	inline void setY(float setY)			{ y = setY;}
	inline void setZ(float setZ)			{ z = setZ;}

	inline float getWidth()					{ return width;}
	inline float getLength()				{ return length;}
	inline float getHeight()				{ return height;}

	inline void setWidth(float setWidth)	{ width = setWidth;}
	inline void setLength(float setLength)	{ length = setLength;}
	inline void setHeight(float setHeight)	{ height = setHeight;}
};


#endif