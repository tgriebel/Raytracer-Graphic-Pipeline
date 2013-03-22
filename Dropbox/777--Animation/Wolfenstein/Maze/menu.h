
#ifndef ___MENU___
#define ___MENU___

//////////////////////////////////////////////////////////////////////////////
//Name: Text Demo															//
//Description: Program will create 3 text									//
//			   boxes and allow the user to enter up to 5 characters in each //
//Author: Thomas Griebel and Jack Regan													//
//Date Last Modified: 12/16/2010											//
//Date Started: 10/28/2010													//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include<iostream>
#include <string>
using namespace std;

#include <GL/glut.h>

#define BACKSPACE 0x08
#define DEL 0x7F

GLsizei wh = 500, ww = 500; /* initial window size */


short textboxes = 5;
struct textbox;
struct button;
textbox* tboxes;
TextureID menuGph;

//bool noenemies = false;

void disp2();
void checkValidity(textbox& tbox);
void myReshape2(int w, int h);
void writeText(const string& text, int x, int y);
void mouse2(int btn, int state, int x, int y);
void key2(unsigned char key, int x, int y);
void loadMainMenuGraphic(){
	raw_bmp* image = new raw_bmp("mainmenu.bmp");
	loadTexture32(image, menuGph);
	delete image;
	image = NULL;
}
void loadGameOverGraphic(){
	raw_bmp* image = new raw_bmp("gameover.bmp");
	loadTexture32(image, menuGph);
	delete image;
	image = NULL;
}
struct textbox{

	void (*action)(void);

	typedef unsigned short uint16;
	bool selected;
	bool error_tag;
	bool dynamic;
	bool isbutton;

	long maxchars;
	string contents;

	uint16 x; uint16 y; 
	uint16 width; uint16 height;

	textbox(){}
	textbox(uint16 xi, uint16 yi, uint16 widthi, uint16 heighti):
		x(xi), y(yi), width(widthi), height(heighti), selected(false), contents(""), 
			maxchars(2), error_tag(false), dynamic(true), isbutton(false){}

	void init(uint16 xi, uint16 yi, uint16 widthi, uint16 heighti, uint32 maxcharsi = 2){
		x = xi, y = yi, width = widthi, height = heighti,selected = false, contents = ""
			, maxchars = maxcharsi, error_tag = false, dynamic = true, isbutton = false;
	}
	
	void init_func(void (*func)()){
		action = func;
	}

	void draw(){
		if (selected)	{
			if(error_tag)	{glColor3f(1.0,0.5,0.5);}
			else			{glColor3f(1.0,1.0,1.0);}
		
		}
		else{			
			if(error_tag)	{glColor3f(1.0,0.2,0.2);}
			else			{glColor3f(0.7,0.7,0.7);}

		}


		glBegin(GL_POLYGON);
			glVertex3f(x,y,0);
			glVertex3f(x+width,y,0);
			glVertex3f(x+width,y+height,0);
			glVertex3f(x,y+height,0);
		glEnd();

		glPushMatrix();

		glTranslatef(x+2,y+height/4,0);
		glScalef(0.25, 0.25, 0.25);

		glColor3f(0.0,0.0,0.0);
		for (int i =0; i<contents.length(); ++i)
			glutStrokeCharacter(GLUT_STROKE_ROMAN, contents.c_str()[i]);
		glPopMatrix();

	}

};

void myReshape2(int w, int h)
{
	glutReshapeWindow(ww,wh);
        //ww = w;
        //wh = h; 
}
void writeText(const string& text, int x, int y){

	glPushMatrix();

	glTranslatef(x+2,y/4,0);
	glScalef(0.25, 0.25, 0.25);

	glColor3f(0.0,0.0,0.0);
	for (int i =0; i<text.length(); ++i)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, text.c_str()[i]);
	glPopMatrix();

}
void checkValidity(textbox& tbox){

	short num_val = atoi(tbox.contents.c_str());
	if(num_val > 20 || num_val < 2){
		tbox.error_tag = true;
		cerr<<"Not in proper range for maze size (2-20)"<<endl;
	}else{
		tbox.error_tag = false;
	}
}
void exitgame(){
	exit(0);
}
void enemyCheckBox(){

	noenemies ^= true;
}
void makeTboxesGameOver(){
	loadGameOverGraphic();
	textboxes = 3;
	tboxes = new textbox[textboxes];
	/* set clear color to grey */
    glClearColor(1.0, 0.5, 0.5, 1.0);
	
	tboxes[0].init(245,150,135,30);
	tboxes[0].init_func(enemyCheckBox);
	tboxes[0].dynamic = false;
	tboxes[0].isbutton = true;

	tboxes[1].init(245,220,150,30);
	tboxes[1].init_func(myInit);
	tboxes[1].dynamic = false;
	tboxes[1].isbutton = true;

	tboxes[2].init(270,170,100,30);
	tboxes[2].init_func(exitgame);
	tboxes[2].dynamic = false;
	tboxes[2].isbutton = true;
}
void makeTboxesMainMenu(){
	loadMainMenuGraphic();
	textboxes = 2;
	tboxes = new textbox[textboxes];
	/* set clear color to grey */

    glClearColor(0.5, 0.5, 0.5, 1.0);

	tboxes[0].init(245,220,150,30);
	tboxes[0].init_func(myInit);
	tboxes[0].dynamic = false;
	tboxes[0].isbutton = true;

	tboxes[1].init(270,150,100,30);
	tboxes[1].init_func(enemyCheckBox);
	tboxes[1].dynamic = false;
	tboxes[1].isbutton = true;
}

void init2()
{
	glutDisplayFunc(disp2);
    glutReshapeFunc(myReshape2); 
	glutKeyboardFunc(key2);
	glutMouseFunc(mouse2);

	makeTboxesMainMenu();

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); 
    gluOrtho2D(0.0, (GLdouble)ww, 0.0, (GLdouble)wh);
	
    glMatrixMode(GL_MODELVIEW);

    glViewport(0,0,ww,wh);

}

void mouse2(int btn, int state, int x, int y){

	bool valid = true;
	y = wh - y;
	if(btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN){

		
		for(int i(0); i < textboxes; ++i){

			valid &= !tboxes[i].error_tag;

			if(x>= tboxes[i].x && x<= (tboxes[i].width+tboxes[i].x) &&
				y>= tboxes[i].y && y<= (tboxes[i].height+tboxes[i].y))
			{
				if(tboxes[i].dynamic){
					tboxes[i].selected = true;
					int selected = 0;
					tboxes[i].contents.clear();
				}else if(tboxes[i].isbutton){
					tboxes[i].action();
				}

			}
			
			else
			{
				tboxes[i].selected = false;
			}
		}//end loop

	}
	glutPostRedisplay();
}

void disp2()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0,1.0,1.0);
	glBindTexture(GL_TEXTURE_2D, menuGph.ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glBegin(GL_QUADS);
		glTexCoord2f (0.0, 0.0);
		glVertex3f(0,0,1);

		glTexCoord2f(1,0);
		glVertex3f(ww,0,1);
		
		glTexCoord2f(1,1);
		glVertex3f(ww,wh,1);
		
		glTexCoord2f(0,1);
		glVertex3f(0,wh,1);
		
	glEnd();
	glDisable(GL_TEXTURE_2D);

	if(noenemies){
		glColor3f(0,1,0);
		glBegin(GL_QUADS);
			glVertex3f(235,150,0.5);
			glVertex3f(235,180,0.5);
			glVertex3f(205,180,0.5);
			glVertex3f(205,150,0.5);
		glEnd();
	}

	glutSwapBuffers();
}

void key2(unsigned char key, int x, int y)
{

	for(int i(0); i < textboxes; ++i){
		
		if(tboxes[i].selected){
			if (key == BACKSPACE || key == DEL){
				if(tboxes[i].contents.length() > 0)
				{
					tboxes[i].contents.resize(tboxes[i].contents.length()-1);
					
				}
			}
			else{
				if(tboxes[i].contents.length() < tboxes[i].maxchars)
					{tboxes[i].contents += key;}
				
			}
			
			checkValidity(tboxes[i]);
			glutPostRedisplay();
		}

	}

	if(key == ESC){
		exit(1);
	}


}

#endif