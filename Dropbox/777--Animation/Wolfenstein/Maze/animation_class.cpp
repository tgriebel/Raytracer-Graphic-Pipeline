#include "animation_class.h"

Animation::Animation():x(0.0), y(0.0), z(0.0), height(1.0), width(1.0), frame(0), total_frames(6),
		loopEnd(4),loopStart(0), playing(false),looping(false), fps(25)
{}

Animation::Animation(int total_framesi, int fpsi, const TextureID& tTagSet):
	x(0.0), y(0.0), z(0.0),
	height(1.0), width(1.0),
	frame(0), total_frames(total_framesi), fps(fpsi),	
	loopEnd(4),loopStart(0), playing(false),looping(false),
	tTag(tTagSet), clicks(0)	
{}
Animation::Animation(const Animation& ani):
	x(ani.x), y(ani.y), z(ani.z),
	height(ani.height), width(ani.width),
	frame(ani.frame), total_frames(ani.total_frames),
	loopEnd(ani.loopEnd), loopStart(ani.loopStart),
	playing(ani.playing), looping(ani.looping),
	fps(ani.fps), tTag(ani.tTag), clicks(ani.clicks)
{}
Animation& Animation::operator=(const Animation& ani){
	x = ani.x; y = ani.y; z = ani.z;
	height = ani.height; width = ani.width;
	frame = ani.frame; total_frames = ani.total_frames;
	loopEnd = ani.loopEnd; loopStart = ani.loopStart;
	playing = ani.playing; looping = ani.looping;
	fps = ani.fps; tTag = ani.tTag; clicks = ani.clicks;

	return *this;
}

void Animation::setDim(float xi, float yi,float zi, float heighti, float widthi){
	x = xi;
	y = yi;
	z = zi;
	height = heighti;
	width = widthi;
}

void Animation::insertFrame(const Frame& new_frame, int previous_frame, int next_frame)
{
	//first convert to itterators
	if(previous_frame == -1 && previous_frame == -1)
	{
		//insert at the end
	}
	else if(previous_frame == -1)
	{
		//insert into front
	}
	else
	{
		//insert where they want
	}
	//then to the actual insertion
}
void Animation::removeFrame(int frame_number)
{
	
	if(frame_number < 0);
}
	
void Animation::stop(){
	playing = false;
	looping = false;
}
void Animation::loop(unsigned start, unsigned end){
	if(start < end){
		looping = true;
		playing = true;	
		loopStart = start;
		loopEnd	  = end;
		frame = loopStart;
	}else{
		cerr<<"Loop range error."<<endl;
	}

}
void Animation::nextframe(){

	frame++;
	if(looping && (frame == (loopEnd + 1))){
		frame = loopStart;
	}else if(frame == (total_frames + 1)){
		frame = 0;
		playing = false;
	}
}
void Animation::update(){

	if(playing == true && clock()){
		nextframe();
	}
	display();
}
void Animation::play(){
	looping = false;
	playing = true;		
}
void  Animation::setTexture(TextureID& tag_set){
	tTag = tag_set;
}
void  Animation::display(){

//	cout<<frameRange()<<endl;

	glBindTexture(GL_TEXTURE_2D, tTag.ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glColor4f(1.0,1.0,1.0,1.0);

	glBegin(GL_POLYGON);


		double frame_d1 = frame/ (double)total_frames;//start of frame cell
		double frame_d2 = (frame +1)/ (double)total_frames;//end of frame cell
		
		glTexCoord2f(frame_d1,0.0);
		glVertex3f(x-width/2.0,y,z);

		glTexCoord2f(frame_d2,0.0);
		glVertex3f(x+width/2.0,y,z);

		glTexCoord2f(frame_d2,1.0);
		glVertex3f(x+width/2.0,y+height,z);

		glTexCoord2f(frame_d1,1.0);
		glVertex3f(x-width/2.0,y+height,z);


	glEnd();

}

//////////////////////////////////
//		Helper Functions		//
//////////////////////////////////

int Animation::clock(){

	(++clicks) %= fps;
	if(clicks == (fps -1)){
		return 1;
	}else{
		return 0;
	}

}