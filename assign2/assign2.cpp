// assign2.cpp : Defines the entry point for the console application.
//

/*
	CSCI 480 Computer Graphics
	Assignment 2: Simulating a Roller Coaster
	C++ starter code
*/
#include<vector>
#include "stdafx.h"
#include <pic.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <GL/glu.h>
#include <GL/glut.h>
#include <chrono>

/* represents one control point along the spline */
struct point {
	double x;
	double y;
	double z;
};
GLuint texture[1];

struct spline {
	int numControlPoints;
	struct point *points;
};
/* the spline array */
struct spline *g_Splines;
point crSplines(float s, float u, point p1, point p2, point p3, point p4);
void texload(int i, char *filename);
/* total number of splines */
int g_iNumOfSplines;

int g_iMenuId;

int g_vMousePos[2] = { 0, 0 };
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;
int framecount = 0;
float xl = 0.f;
float yl = 1.f;
float zl = 0.f;
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
bool recordstate;
int frames = 0;
GLuint listName;
bool trianglelines;
std::chrono::time_point<std::chrono::high_resolution_clock> start, end, fstart, fend;


CONTROLSTATE g_ControlState = ROTATE;

GLenum g_render;
/* state of the world */
float g_vLandRotate[3] = { 0.0, 0.0, 0.0 };
float g_vLandTranslate[3] = { 0.0, 0.0, 0.0 };
float g_vLandScale[3] = { 1.0, 1.0, 1.0 };

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;

/* Write a screenshot to the specified filename */
void saveScreenshot(char *filename)
{
	int i, j;
	Pic *in = NULL;

	if (filename == NULL)
		return;

	/* Allocate a picture buffer */
	in = pic_alloc(640, 480, 3, NULL);

	printf("File to save to: %s\n", filename);

	for (i = 479; i >= 0; i--) {
		glReadPixels(0, 479 - i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
			&in->pix[i*in->nx*in->bpp]);
	}

	if (jpeg_write(filename, in))
		printf("File saved Successfully\n");
	else
		printf("Error in Saving\n");

	pic_free(in);
}
void enableLights()

{

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}
void drawHeightMap()
{

}
void myinit()
{
	/* setup gl view here */
	glGenTextures(1, texture);
	texload(0, "bubble1.jpg");
	glClearColor(0.0, 0.0, 0.0, 0.0);   // set background color
	glEnable(GL_DEPTH_TEST);            // enable depth buffering
	glShadeModel(GL_SMOOTH); // interpolate colors 

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, .01, 1000);

	glMatrixMode(GL_MODELVIEW);

	drawHeightMap();

}

void reshape(int w, int h)
{
	// setup image size
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// setup camera
	gluPerspective(60, 1, .01, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}



void record()
{
	if (recordstate & frames<300)
	{
		frames++;
		end = std::chrono::high_resolution_clock::now();
		char myFilenm[2048];
		std::chrono::duration<double> elapsed_seconds = end - start;
		if (elapsed_seconds.count() >= 1.0 / 30.0) // does a capture every 1/30 of a second for 30 fps
		{
			start = std::chrono::high_resolution_clock::now();

			sprintf_s(myFilenm, "anim.%04d.jpg", frames);
			saveScreenshot(myFilenm);
			return;
		}
		return;
	}
}
void measurefps()
{
	framecount++;
	fend = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed_seconds = fend - fstart;
	if (elapsed_seconds.count() >= 1.0)
	{
		printf("fps is %d \n", framecount);
		framecount = 0;
		fstart = std::chrono::high_resolution_clock::now();
		//printf("control points is %d \n",g_Splines->numControlPoints);
		//printf("number of splines is %d \n", g_iNumOfSplines);

	}
}
void display()
{
	/* draw 1x1 cube about origin */
	/* replace this code with your height field implementation */
	/* you may also want to precede it with your
	rotation/translation/scaling */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
	glLoadIdentity(); // reset transformation

	glTranslatef(-g_vLandTranslate[0], -g_vLandTranslate[1], -g_vLandTranslate[2]);
	glRotatef(g_vLandRotate[0], 1, 0, 0);
	glRotatef(g_vLandRotate[1], 0, 1, 0);
	glRotatef(g_vLandRotate[2], 0, 0, 1);
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);

	//enableLights();
	float s=0.5; 
	point buff;
	point p0,p1,p2,p3; 
	p0.x = 0;
	p0.y = 0;
	p0.z = 0;
	for (int i = 0; i < g_Splines->numControlPoints; i++)
	{
		for (float u = 0.f; u < 1.f; u += 0.001f)
		{
			glBegin(GL_POINTS);
			p1 = g_Splines->points[i];
			p2 = g_Splines->points[(i + 1) % g_Splines->numControlPoints];
			p3 = g_Splines->points[(i + 2) % g_Splines->numControlPoints];
	
			
			buff = crSplines(s, u, p0, p1, p2, p3);

			glColor3f(1.0,1.0, 1.0);
			glVertex3f(buff.x, buff.y, buff.z);
			glEnd();
			p0 = p1;
		}

	}


	glBegin(GL_LINES);
	glColor3f(1.0, 0, 0);
	glVertex3f(0, 0, 0);
	glColor3f(1.0, 0, 0);
	glVertex3f(0, 0, 100);
	glEnd();


	//green x axis line
	glBegin(GL_LINES);
	glColor3f(0, 1.0, 0);
	glVertex3f(0, 0, 0);
	glColor3f(1.0, 1.0, 1.0);
	glVertex3f(100, 0, 0);
	glEnd();

	//blue y axis line
	glBegin(GL_LINES);
	glColor3f(0, 0, 1.0);
	glVertex3f(0, 0, 0);
	glColor3f(0, 0, 1.0);
	glVertex3f(0, 100, 0);
	glEnd();
	glEndList();
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);
	
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0, 1.0, 1.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, 1.0, 1.0);
	
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0, 1.0, -1.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0, 1.0, -1.0);
	
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glutSwapBuffers();
	record();

}

void menufunc(int value)
{
	switch (value)
	{
	case 0:
		exit(0);
		break;
	}
}

void doIdle()
{
	/* do some stuff... */

	/* make the screen update */
	//Sleep(1);//program ran really slow without using this; 
	record();
	measurefps();

	glutPostRedisplay();
}

/* converts mouse drags into information about
rotation/translation/scaling */
void mousedrag(int x, int y)
{
	int vMouseDelta[2] = { x - g_vMousePos[0], y - g_vMousePos[1] };

	switch (g_ControlState)
	{
	case TRANSLATE:
		if (g_iLeftMouseButton)
		{
			g_vLandTranslate[0] += vMouseDelta[0] * 0.1;
			g_vLandTranslate[1] -= vMouseDelta[1] * 0.1;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandTranslate[2] += vMouseDelta[1] * 0.1;
		}
		break;
	case ROTATE:
		if (g_iLeftMouseButton)
		{
			g_vLandRotate[0] += vMouseDelta[1];
			g_vLandRotate[1] += vMouseDelta[0];
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandRotate[2] += vMouseDelta[1];
		}
		break;
	case SCALE:
		if (g_iLeftMouseButton)
		{
			g_vLandScale[0] *= 1.0 + vMouseDelta[0] * 0.01;
			g_vLandScale[1] *= 1.0 - vMouseDelta[1] * 0.01;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandScale[2] *= 1.0 - vMouseDelta[1] * 0.01;
		}
		break;
	}
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		g_iLeftMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_MIDDLE_BUTTON:
		g_iMiddleMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_RIGHT_BUTTON:
		g_iRightMouseButton = (state == GLUT_DOWN);
		break;
	}

	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		g_ControlState = TRANSLATE;
		break;
	case GLUT_ACTIVE_SHIFT:
		g_ControlState = SCALE;
		break;
	default:
		g_ControlState = ROTATE;
		break;
	}

	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}
void startrecord()
{
	recordstate = true;
	start = std::chrono::high_resolution_clock::now();
	frames = 0;
}

void keyboardbuttons(unsigned char c, int x, int y)
{
	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_ALT:
	{
		switch (c)
		{
		case 'p':
		{
			g_render = GL_POINTS;
			trianglelines = false;
			drawHeightMap();
			break;
		}
		case 'l':
		{
			g_render = GL_LINES;
			trianglelines = false;

			drawHeightMap();
			break;
		}
		case 't':
		{
			g_render = GL_TRIANGLE_STRIP;
			trianglelines = false;

			drawHeightMap();
			break;
		}
		case 'h':
		{
			//returns "camera" to starting position
			g_vLandRotate[0] = 0;
			g_vLandRotate[1] = 0;
			g_vLandRotate[2] = 0;

			g_vLandTranslate[0] = 0;
			g_vLandTranslate[1] = 0;
			g_vLandTranslate[2] = 0;

			g_vLandScale[0] = 1.0;
			g_vLandScale[1] = 1.0;
			g_vLandScale[2] = 1.0;

			break;
		}
		case 'a': //starts recording screencaps for movie
			startrecord();
			break;
		case 'b':
			g_render = GL_TRIANGLE_STRIP;
			trianglelines = true;
			drawHeightMap();

			break;

		}

		break;
	}
	}
}

/* spline struct which contains how many control points, and an array of control points */
void texload(int i, char *filename)
{
	Pic* img;
	img = jpeg_read(filename, NULL);
	glBindTexture(GL_TEXTURE_2D, texture[i]);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		img->nx,
		img->ny,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		&img->pix[0]);
	pic_free(img);
}



int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;

	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf ("can't open file\n");
		exit(1);
	}
  
	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);

	g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf ("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
		g_Splines[j].numControlPoints = iLength;

		/* saves the data to the struct */
		while (fscanf(fileSpline, "%lf %lf %lf", 
			&g_Splines[j].points[i].x, 
			&g_Splines[j].points[i].y, 
			&g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

point crSplines(float s,float u, point p1, point p2,point p3, point p4)
{

	point output;
	float u2 = u*u;
	float u3 = u*u*u;
	float a = -s*u + 2.f * s*u*u- s*u*u*u;
	float b = 1.f + (-3.f + s)*u*u + (2.f - s)*u*u*u;
	float c = s*u + (3.f - 2.f* s)*u*u + (-2.f + s)*u*u*u;
	float d = -s*u*u + s*u*u*u;

	output.x = a*(float)p1.x + b*(float)p2.x + c*(float)p3.x + d*(float)p4.x;
	output.y = a*(float)p1.y + b*(float)p2.y + c*(float)p3.y + d*(float)p4.y;
	output.z = a*(float)p1.z + b*(float)p2.z + c*(float)p3.z + d*(float)p4.z;

	return output;

}

int _tmain(int argc, _TCHAR* argv[])
{
	// I've set the argv[1] to track.txt.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your track file name for the "Command Arguments"
	if (argc<2)
	{  
		printf ("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	loadSplines(argv[1]);

	fstart = std::chrono::high_resolution_clock::now();

	/*
	create a window here..should be double buffered and use depth testing

	the code past here will segfault if you don't have a window set up....
	replace the exit once you add those calls.
	*/
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

	// set window size
	glutInitWindowSize(680, 480);

	// set window position
	glutInitWindowPosition(0, 0);

	// creates a window
	glutCreateWindow("Assign One");

	/* allow the user to quit using the right mouse button menu */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Quit", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	/* replace with any animate code */
	glutIdleFunc(doIdle);
	myinit();

	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);

	glutKeyboardFunc(keyboardbuttons);
	glutReshapeFunc(reshape);

	/* do initialization */
	glutDisplayFunc(display);



	glutMainLoop();
	return 0;
}