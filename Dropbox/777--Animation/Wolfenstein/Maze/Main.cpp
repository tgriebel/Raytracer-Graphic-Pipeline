#include <iostream>
#include <time.h>
#include <sstream>
#include <cstring>
#include <cctype>
using namespace std;

#include <gl\glut.h>

#include <iostream>
using namespace std;

#include <gl\glut.h>
#include <vector>

#define ESC 0x1B

#include "texture.h"
//#include "animation.h"
#include "UI.h"
#include "global_prototypes.h"
#include "FreeCamera.h"

unsigned height(63), width(57);

int start_x(-1), start_z(-1);
int end_cell_x(-1), end_cell_z(-1);
int at_x(1), at_z(1);

bool quit = false;
const int CANDELABRA=1;

void myInit();
bool noenemies = false;
#include "menu.h"

struct tile{

	int x, z;

	bool walkable;
	bool door;
	bool exit;
	char type;

	GLuint texture[6];
};

unsigned gW, gH;
const unsigned TIMER_MS = 25;// 25 milliseconds makes the display update every 24 times/second
const unsigned draw_dist = 15;
const unsigned mheight(63), mwidth(57);

UI *ui;

tile game_map[mheight][mwidth];

FreeCamera* camera;//use reference to change between free cam and target cam easily

bool cullback = true;
bool SPACE_TOGGLE = false;
bool wireframe_flag = false;

TextureID checkerTex;

void specialKeys(int key, int x, int y);
void myKeyboard(unsigned char key, int x, int y);
void update(int value);
void mouseAim(int x2, int y2);
void mouse(int btn, int state, int x, int y);

void display(){}
void clearWindow(){
	glClearColor(0.0, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PerspectiveSet(int w, int h, float fov){
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glViewport(0, 78, w, h);
	gluPerspective(fov,(w/(h*1.0)),.01,draw_dist);
    
    glMatrixMode(GL_MODELVIEW);
}
void OrthoSet(int w, int h){}
void myReshape(int w, int h)
{

	gW = w;
	gH = h;

	PerspectiveSet(w, h, camera->fov);

	ui->resize(gW, gH);

}

void drawCube(float x, float y, float z, float size){

	float color[4] = {1.0,0.0,0.0,0.5};
	glColor4fv(color);

	glNormal3f(1.0,0.0,0.0);
	glBegin(GL_QUADS);
		glVertex3f(x+size, 0, z);
		glVertex3f(x+size, 0, z+size);
		glVertex3f(x+size, y, z+size);
		glVertex3f(x+size,y,z);	
	glEnd();

	glNormal3f(-1.0,0.0,0.0);
	glBegin(GL_QUADS);
		glVertex3f(x, y, z+size);
		glVertex3f(x, 0, z+size);
		glVertex3f(x, 0, z);
		glVertex3f(x,y,z);
	glEnd();

	glNormal3f(0.0,0.0,-1.0);
	glBegin(GL_QUADS);
		glVertex3f(x+size, y, z+size);
		glVertex3f(x+size, 0, z+size);
		glVertex3f(x, 0, z+size);
		glVertex3f(x,y,z+size);	
	glEnd();
	
	glNormal3f(0.0,0.0,1.0);
	glBegin(GL_QUADS);
		glVertex3f(x, 0, z);
		glVertex3f(x+size, 0, z);
		glVertex3f(x+size, y, z);
		glVertex3f(x,y,z);
	glEnd();

	glNormal3f(0.0,-1.0,0.0);
	glBegin(GL_QUADS);
		glVertex3f(x, 0, z);
		glVertex3f(x+size, 0, z);
		glVertex3f(x+size, 0, z+size);
		glVertex3f(x,0,z+size);
	glEnd();

	glNormal3f(0.0,1.0,0.0);
	glBegin(GL_QUADS);
		glVertex3f(x, y, z);
		glVertex3f(x+size, y, z);
		glVertex3f(x+size, y, z+size);
		glVertex3f(x,y,z+size);
	glEnd();

}

void myInit()
{
	cullback = true;
	SPACE_TOGGLE = false;
	wireframe_flag = false;
	quit = false;

	glClearColor(1.0, .5, .5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutDisplayFunc(display);
	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(specialKeys);
	glutReshapeFunc(myReshape);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(mouseAim);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);


	raw_bmp* image = new raw_bmp("UI.bmp");

	TextureID textID;
	loadTexture32(image, textID);
	
	ui = new UI(gH, gW, textID);
	
	delete image;	image = NULL;

	image = new raw_bmp("checker.bmp");
	loadTexture32(image, checkerTex);
	delete image;
	image = NULL;

	camera = new FreeCamera(.2,5.0,start_x + 0.5,0.75,start_z +0.5,0.0,-180.0);
	
	glutTimerFunc(TIMER_MS, update, 0);
	glutPostRedisplay();
}

void mouse(int btn, int state, int x, int y){

	y = wh - y;
	if(btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN){	
	}
}
void mouseAim(int x2, int y2){

	if(SPACE_TOGGLE){

		glutSetCursor (GLUT_CURSOR_NONE);
			
		static int x1(0), y1(0);
		static int resetCnt(0);

		float dx = (float)x2 - (float)(gW/2.0);
		float dy = (float)y2 - (float)(gH/2.0);

		float theshold = 8;

		if(resetCnt > 1){//100 cycles

			if(dx > 0 && dx > theshold){
				camera->panX(-2);
			}else if (dx < -theshold){
				camera->panX(2);
			}

			if(dy < 0  && dy < -theshold){
				camera->panY(2);
			}else if (dy > theshold){
				camera->panY(-2);
			}

			glutWarpPointer  ( gW/2.0 , gH/2.0 );
			resetCnt = 0;
		
		}else{
			resetCnt++;
		}
	}else{
		glutSetCursor (GLUT_CURSOR_CROSSHAIR);
	}
}
void specialKeys(int key, int x, int y){

	switch(key){
		case GLUT_KEY_UP:
			camera->truckUp();
			break;
		case GLUT_KEY_DOWN:
			camera->truckDown();
			break;
		case GLUT_KEY_LEFT:
			camera->truckLeft();
			break;
		case GLUT_KEY_RIGHT:
			camera->truckRight();
			break;
		case GLUT_KEY_F1:
			cullback ^= true;
			break;
		case GLUT_KEY_F2:
			wireframe_flag ^= true;
			break;
	}
}

void myKeyboard(unsigned char key, int x, int y){

	if(key == ESC) quit = true;

	tolower(key);
	switch(key){
	
		case 32:
			SPACE_TOGGLE  ^= true;
			break;
		case 'a':
			camera->panX(2);
			break;
		case 'd':
			camera->panX(-2);
			break;
		case 'r':
			camera->panY(2);
			break;
		case 'f':
			camera->panY(-2);
			break;
		case 's':
			camera->dollyBack();
			break;
		case 'w':
			camera->dollyForward();
			break;
		case 'q':
			camera->roll(2.);
			break;
		case 'e':
			camera->roll(-2.0);
			break;
		case 't':
			camera->dollyTargetForward();
			break;
		case 'g':
			camera->dollyTargetBack();
			break;
		case 'y':
			camera->dollyCameraForward();
			break;
		case 'h':
			camera->dollyCameraBack();
			break;
		case '[':
			camera->orbitX(-2.);
			break;
		case ']':
			camera->orbitX(2.);
			break;
		case 'p':
			camera->orbitY(2.);
			break;
		case ';':
			camera->orbitY(-2.);
			break;
		case 'v':
			camera->fov++;
			break;
		case 'b':
			camera->fov--;
			break;
		default:
			break;
	}
}
void update(int value) {
	
	if(quit){
		return;
	}else{
		glutTimerFunc(TIMER_MS, update, 0);
	}

	PerspectiveSet(gW, gH, camera->fov);

	glEnable(GL_FOG);
	GLfloat fogColor[] = {0.5f, 0.6f, 0.4f, 1};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 0.0f);
    glFogf(GL_FOG_END, draw_dist);

	if(cullback) glCullFace(GL_BACK);
	else		 glCullFace(GL_FRONT);

	if(wireframe_flag)					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else								glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(fogColor[0], fogColor[1], fogColor[2], fogColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(camera->pos[0],camera->pos[1],camera->pos[2],
		  camera->at[0],camera->at[1],camera->at[2],
		  camera->up[0],camera->up[1],camera->up[2]);

	//drawCube(camera->at[0], camera->at[1], camera->at[2], 0.1);

	glBindTexture(GL_TEXTURE_2D, checkerTex.ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) ;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable( GL_TEXTURE_2D );
	glNormal3f(0.0,1.0,0.0);
	
	float floor_color[4] = {1.0, 1.0, 1.0,1.0};
	glColor4fv( floor_color);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, floor_color);

	glBegin(GL_POLYGON);
		
		glTexCoord2f (0.0, 0.0);
		glVertex3f(-30,0,-30);

		glTexCoord2f (0.0, 8.);
		glVertex3f(-30,0, 30);

		glTexCoord2f (8., 8.);
		glVertex3f(30, 0, 30);

		glTexCoord2f (8., 0.0);
		glVertex3f(30, 0, -30);
	glEnd();

	glDisable( GL_TEXTURE_2D );

	draw2DModeON();

	draw2DModeOFF();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0.0, (GLdouble)gW, 0.0, (GLdouble)gH, .1, draw_dist);
	glViewport(0, 0, gW, gH);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_FOG);

	ui->draw();

	glutSwapBuffers();
}
void main(int argc, char **argv)
{
	//seed the random number generator to give different results each time
	srand(time(0));

	ww = 640;
	wh = 480;

	gW = 640;
	gH = 480;

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB |GLUT_DEPTH);
    glutInitWindowSize(gW, gH);
    glutCreateWindow("Mocap Viewer");

	init2();

	glutMainLoop();
}
