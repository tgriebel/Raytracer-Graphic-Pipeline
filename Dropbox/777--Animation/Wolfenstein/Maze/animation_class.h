//////////////////////////////////////////////////////////////////////////////
//Name: Animation Class														//
//Description: Will manage individual frames and will play them according	//
//				to its clock												//
//Author: Thomas Griebel													//
//Date Last Modified: 12/23/2010											//
//Date Started: 11/24/2010													//
//////////////////////////////////////////////////////////////////////////////

#ifndef ___ANIMATION_CLASS___
#define ___ANIMATION_CLASS___
#include "TextureStructs.h"
#include "Frame.h"
#include <iostream>
#include <vector>
using namespace std;
#include <gl\glut.h>
#define TIMER 25
class Animation{
//Animations are not intended to be used outside of other classes (i.e. friends, children)

private:
	float height;
	float width;

	unsigned frame;
	unsigned total_frames;

	unsigned loopStart;
	unsigned loopEnd;
	bool looping;
	bool playing;

	TextureID tTag;
	vector<Frame*> frames;

	unsigned fps;
	int clicks;
	int clock();
	
	void display();
protected:
	float x;
	float y;
	float z;
public:

	friend class Creature;

	Animation();
	Animation(int total_framesi,int fpsi, const TextureID& tTagSet);
	Animation(const Animation& ani);
	Animation& operator=(const Animation&);

	inline int getX()						{return x;				}
	inline int getY()						{return y;				}
	inline int getZ()						{return z;				}
	inline unsigned getWidth()				{return width;			}
	inline unsigned getHeight()				{return height;			}
	inline void setWidth(int widthset)		{width = widthset;		}
	inline void setHeight(int heightset)	{height = heightset;	}
	inline unsigned currentFrame()			{return frame;			}
	inline unsigned totalFrames()			{return total_frames;	}
	inline bool isLooping()					{return looping;		}
	inline bool isPlaying()					{return playing;		}

	
	void setDim(float xi, float yi,float zi, float heighti, float widthi);
	void setTexture(TextureID& tag_set);//deprecated

	void insertFrame(const Frame& new_frame, int previous_frame = -1, int next_frame = -1);
	void removeFrame(int frame_number);

	void stop();
	void loop(unsigned start, unsigned end);
	void nextframe();
	void update();
	void play();

	

};

#endif;