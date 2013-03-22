#include <stdio.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <gl/glut.h>
#include "MathVector.h"
#include "BVHParser.h"

using namespace std;

int windowWidth, windowHeight = 512;
bool drawCoordSystem = true;

void DrawCoordinateSystem()
{
	glColor3d(0.0, 0.0, 255.0);
	glBegin(GL_LINES);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(2.0, 0.0, 0.0);
	glEnd();

	glColor3d(0.0, 255.0, 0.0);
	glBegin(GL_LINES);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 2.0, 0.0);
	glEnd();

	glColor3d(255.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 2.0);
	glEnd();
}

void init ( GLvoid )     // Create Some Everyday Functions
{

  glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
   glEnable ( GL_COLOR_MATERIAL );
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void KeyboardFunc(unsigned char c, int x, int y)
{

	switch (c)
	{
	case 27:
	case 'x':
		//glutLeaveMainLoop();
		return;

	case 'c':
		drawCoordSystem = !drawCoordSystem;
		break;
	}
	glutPostRedisplay();
}

void Display ( void )   // Create The Display Function
{

	BVHParser parser;
	NODE* figure = parser.getFigure("Example1.bvh");
	vector<float> motionData = parser.getMotions("Example1.bvh");
	
	double elapsed_time = double(glutGet(GLUT_ELAPSED_TIME)) / 1000.0;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	
	// Reset the display
	glLoadIdentity();

	// Lights
	static GLfloat light_position[] = { 0.0 , 5.0 , 5.0 , 1.0 };
	glLightfv(GL_LIGHT0 , GL_POSITION , light_position);

	gluLookAt(2.0 , 2.0 , 7.0 , 0.0 , 0.0 , 0.0 , 0.0 , 1.0 , 0.0);
	// Action
	if (drawCoordSystem)
	{
		DrawCoordinateSystem();
	}

	int dof = figure->DOF;

	for(int i = 0; i<dof; i++)
	{
	//	motionData[
	}

	glutSwapBuffers ( );
  // Swap The Buffers To Not Be Left With A Clear Screen
}

void ReshapeFunc(int w, int h)
{
	if (h <= 0) return;

	windowWidth = w;
	windowHeight = h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0 , ((double) w) / ((double) h) , 2.0 , 50.0);
	glViewport(0 , 0 , w , h);
}

int main( int argc, char* argv[] )
{
	BVHParser parser;
	NODE* figure = parser.getFigure("Example1.bvh");
	vector<float> motionData = parser.getMotions("Example1.bvh");
	
	/*NODE* figure = parser.getFigure("03_01.bvh");
	vector<float> motionData = parser.getMotions("03_01.bvh");
	*/

	/*
	glutInit(&argc , argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(0 , 0);
	glutCreateWindow("Mocap");

	glEnableClientState(GL_NORMAL_ARRAY);

	glutDisplayFunc(Display);
	glutReshapeFunc(ReshapeFunc);
	glutKeyboardFunc(KeyboardFunc);

	glClearColor(0.0 , 0.0 , 0.0 , 0.0);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE , 0.0);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER , 1.0);
	glutMainLoop();

	return 0;
	*/
}






