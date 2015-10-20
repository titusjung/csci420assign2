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
#include <math.h>

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
point getUnit(point p1, point p2);
void takeRide();
point arithVector(bool isaPos, point a, bool isbPos, point b);

point getTangent(float s, float u, point p1, point p2, point p3, point p4);

void texload(int i, char *filename);
/* total number of splines */
int g_iNumOfSplines;

int g_iMenuId;

bool isRCrun = false;
int rideControlPoint=0;
float rideFP=0.f;

point rtan;
point rp0, rp1, rp2, rp3;
point binormalOld;

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

	if (isRCrun) takeRide();
	
	glTranslatef(-g_vLandTranslate[0], -g_vLandTranslate[1], -g_vLandTranslate[2]);
	glRotatef(g_vLandRotate[0], 1, 0, 0);
	glRotatef(g_vLandRotate[1], 0, 1, 0);
	glRotatef(g_vLandRotate[2], 0, 0, 1);
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
	
	//enableLights();
	float s=0.5f; 
	point buff;
	point p0,p1,p2,p3; 
	p0 = g_Splines->points[0];
	glLineWidth(1.5f);

	point n0, b0, b1, v, t0,v0,v1,v2,v3,v4,v5,v6,v7;
	bool start = false;
	v.x = 1.f;
	v.y = 1.f;
	v.z = 1.f;
	t0 = getTangent(0.5f, 0, p0, g_Splines->points[0], g_Splines->points[1], g_Splines->points[2]);
	n0 = getUnit(t0, v);
	b0 = getUnit(t0, n0);
	for (int i = 0; i < g_Splines->numControlPoints; i++)
	{

		p1 = g_Splines->points[i];
		p2 = g_Splines->points[(i + 1) % g_Splines->numControlPoints];
		p3 = g_Splines->points[(i + 2) % g_Splines->numControlPoints];
		glBegin(GL_LINES);

		for (float u = 0.f; u < 1.f; u += 0.1f)
		{


	
			
			buff = crSplines(s, u, p0, p1, p2, p3);
			t0 = getTangent(s, u, p0, p1, p2, p3);
			glColor3f(1.0,1.0, 1.0);
			glVertex3f(buff.x, buff.y, buff.z);

			n0 = getUnit(b0, t0);
			b1 = getUnit(t0, n0);

			if(start)
			{
				v4 = arithVector(true,buff, true,  arithVector(true, n0, false, b0));
				v5 = arithVector(true, buff, true, arithVector(true, n0, true, b0));
				v6 = arithVector(true, buff, true, arithVector(false, n0, true, b0));
				v7 = arithVector(true, buff, true, arithVector(false, n0, false, b0));

				glBegin(GL_POLYGON);
				glColor3f(1.0, 1.0, 1.0);
				glVertex3f(v0.x, v0.y, v0.z);

				glColor3f(1.0, 1.0, 1.0);
				glVertex3f(v4.x, v4.y, v4.z);


				glColor3f(1.0, 1.0, 1.0);
				glVertex3f(v5.x, v5.y, v5.z);

				glColor3f(1.0, 1.0, 1.0);
				glVertex3f(v1.x, v1.y, v1.z);
				glEnd();



				v0 = v4;
				v1 = v5;
				v2 = v6;
				v3 = v7;
			}
			else
			{
				start = true;

				v4 = arithVector(true, buff, true, arithVector(true, n0, false, b0));
				v5 = arithVector(true, buff, true, arithVector(true, n0, true, b0));
				v6 = arithVector(true, buff, true, arithVector(false, n0, true, b0));
				v7 = arithVector(true, buff, true, arithVector(false, n0, false, b0));

				v0 = v4;
				v1 = v5;
				v2 = v6;
				v3 = v7;
			}



		}
		p0 = p1;
		glEnd();


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
	glVertex3f(5.0, 5.0, 5.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-5.0, 5.0, 5.0);
	
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-5.0, 5.0, -5.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(5.0, 5.0, -5.0);
	
	glEnd();


	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);

	glTexCoord2f(1.0, 0.0);
	glVertex3f(5.0, -5.0, 5.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-5.0, -5.0, 5.0);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(-5.0, -5.0, -5.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(5.0, -5.0, -5.0);

	glEnd();


	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);

	glTexCoord2f(1.0, 0.0);
	glVertex3f(5.0, 5.0, 5.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(5.0, 5.0, -5.0);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(5.0, -5.0, -5.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(5.0, -5.0, 5.0);

	glEnd();

		glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);

	glTexCoord2f(1.0, 0.0);
	glVertex3f(-5.0, 5.0, 5.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-5.0, 5.0, -5.0);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(-5.0, -5.0, -5.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(-5.0, -5.0, 5.0);

	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);

	glTexCoord2f(1.0, 0.0);
	glVertex3f(5.0, 5.0, 5.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-5.0, 5.0, 5.0);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(-5.00, -5.0, 5.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(5.0, -5.0, 5.0);

	glEnd();


	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);

	glTexCoord2f(1.0, 0.0);
	glVertex3f(5.0, 5.0, -5.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-5.0, 5.0, -5.0);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(-5.0, -5.0, -5.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(5.0, -5.0, -5.0);

	glEnd();

	glDisable(GL_TEXTURE_2D);

	glutSwapBuffers();
	record();

}

void takeRide()
{

	point rtan;
	float s = 0.5f;
	if (rideFP > 1.f)
	{
		rideControlPoint++;
		rideFP = 0.0f;
		printf("control Point %d \n", rideControlPoint);
		rp0 = rp1 = g_Splines->points[rideControlPoint-1];


	}
	if (rideControlPoint == g_Splines->numControlPoints)
	{
		isRCrun = false;
		rideControlPoint = 0;
		rideFP = 0.f;
		return;
	}
	if (rideControlPoint == 0)
	{		

		rp0 = g_Splines->points[g_Splines->numControlPoints - 1];
	}

	
	rp1 = g_Splines->points[rideControlPoint];
	rp2 = g_Splines->points[(rideControlPoint + 1) % g_Splines->numControlPoints];
	rp3 = g_Splines->points[(rideControlPoint + 2) % g_Splines->numControlPoints];

	point currentp = crSplines(s, rideFP, rp0, rp1, rp2, rp3);
	rtan = getTangent(s, rideFP, rp0, rp1, rp2, rp3);

	if (rideFP==0.5f)printf("current locatin is  %f %f %f \n", currentp.x, currentp.y, currentp.z);
	point binormalNew, norm;


	if (rideFP==0.0f && rideControlPoint == 0)
	{
		//printf("initializing \n");

		point vec;
		vec.x = 0.0;
		vec.y = 1.0;
		vec.z = -1.0;
		norm = getUnit(rtan, vec);
		binormalNew = getUnit(rtan, norm);

	}
	else
	{
		norm = getUnit(binormalOld, rtan);
		binormalNew = getUnit(rtan, norm);

	}
	binormalOld = binormalNew;

	float focusPx = currentp.x + 100.f*rtan.x;
	float focusPy = currentp.y + 100.f*rtan.y;
	float focusPz = currentp.z +100.f*rtan.z;

	gluLookAt(currentp.x, currentp.y, currentp.z, focusPx, focusPy, focusPz, norm.x, norm.y, norm.z);


	rideFP += 0.1f;
	
}

point crSplines(float s, float u, point p1, point p2, point p3, point p4)
{

	point output;
	float u2 = u*u;
	float u3 = u*u*u;
	float a = -s*u + 2.0f * s*u*u - s*u*u*u;
	float b = 1.f + (-3.0f + s)*u*u + (2.0f - s)*u*u*u;
	float c = s*u + (3.0f - 2.0f* s)*u*u + (-2.0f + s)*u*u*u;
	float d = -s*u*u + s*u*u*u;

	output.x = (float)a*p1.x + (float)b*p2.x + (float)c*p3.x + (float)d*p4.x;
	output.y = (float)a*p1.y + (float)b*p2.y + (float)c*p3.y + (float)d*p4.y;
	output.z = (float)a*p1.z + (float)b*p2.z + (float)c*p3.z + (float)d*p4.z;

	return output;

}

point getTangent(float s, float u, point p1, point p2, point p3, point p4)
{

	point output;
	float u2 = u*u;
	float a = -3.f * u2 + 4.f * u*s - s;
	float b = 3.f*u2*(2.f - s) + 2.f*u*(s - 3.f);
	float c = 3.f*u2*(s - 2.f) + 2.f*u*(3.f - 2.f*s) + s;
	float d = 3.f*u2 - 2.f*u*s;

	output.x = a*p1.x +b*p2.x + c*p3.x + d*p4.x;
	output.y = a*p1.y + b*p2.y + c*p3.y +d*p4.y;
	output.z = a*p1.z + b*p2.z +c*p3.z +d*p4.z;

	return output;

}

point getUnit(point p1, point p2)
{
	float a1 = p1.x;
	float a2 = p1.y;
	float a3 = p1.z;

	float b1 = p2.x;
	float b2 = p2.y;
	float b3 = p2.z;

	float c1 = -a3 *b2 + a2 *b3;
	float c2 = a3 *b1 - a1 *b3;
	float c3 = -a2 *b1 + a1 *b2;

	float un = sqrtf(c1*c1 + c2*c2 + c3*c3);

	point result;
	result.x = c1 / un;
	result.y = c2 / un;
	result.z = c3 / un;

	return result;
}

point arithVector(bool isaPos, point a,bool isbPos, point b)
{
	point result;
	float signA, signB;
	if (isaPos)
	{
		signA = 1.f;
	}
	else
	{
		signA = -1.f;
	}
	if (isbPos)
	{
		signB = 1.f;
	}
	else
	{
		signB = -1.f;
	}
	result.x = signA*a.x + signB*b.x;
	result.y = signA* a.y + signB*b.y;
	result.z = signA*a.z + signB*b.z;

	return result;
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
			{
				startrecord();
				break;
			}
			case 'b':
			{
				g_render = GL_TRIANGLE_STRIP;
				trianglelines = true;
				drawHeightMap();

				break;
			}
			case'r':
			{
				printf("activating isRCrun\n");
				isRCrun = true;
				break;
			}
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