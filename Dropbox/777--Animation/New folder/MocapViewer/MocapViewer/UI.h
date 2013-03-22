
#ifndef ___ANIMATION_CLASSES___
#define ___ANIMATION_CLASSES___
#include "TextureStructs.h"
struct UI{

	UI(int hi, int wi, const TextureID& tagSet):
		height(hi), width(wi), tTag(tagSet){}

	void resize(int ws, int hs){
		height = hs;
		width  = ws;
	}
	void draw(){

		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, tTag.ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		float color[4] = {1.0, 1.0, 1.0,1.0};
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		//glColor4f(1.0,1.0,1.0,1.0);
		glBegin(GL_POLYGON);
			
			glTexCoord2f(0.0,0.0);
			glVertex3f(0,0,-10);

			glTexCoord2f(1.0,0.0);
			glVertex3f(width,0,-10);

			glTexCoord2f(1.0,1.0);
			glVertex3f(width, height/6.0,-10);

			glTexCoord2f(0.0,1.0);
			glVertex3f(0,height/6.0,-10);

		glEnd();

		glDisable(GL_TEXTURE_2D);

		GLfloat dot_color[4] = {1.0, 0.0, 0.0,0.7};
		glColor4fv(dot_color);
		int size = 4;
		glBegin(GL_POLYGON);
			
			glTexCoord2f(0.0,0.0);
			glVertex3f(width/2. - size,height/2. - size,-10);

			glTexCoord2f(1.0,0.0);
			glVertex3f(width/2. +size,height/2. - size,-10);

			glTexCoord2f(1.0,1.0);
			glVertex3f(width/2. + size, height/2. + size,-10);

			glTexCoord2f(0.0,1.0);
			glVertex3f(width/2. - size,height/2. + size,-10);

		glEnd();

	}

	private:
		int x;
		int y;
		int height;
		int width;
		TextureID tTag;
};
#endif