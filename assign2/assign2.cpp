// assign2.cpp : Defines the entry point for the console application.
//

/*
	CSCI 480 Computer Graphics
	Assignment 2: Simulating a Roller Coaster
	C++ starter code
*/
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/vec4.hpp>
#include <glm/mat4x3.hpp>
#include <glm/vec3.hpp>
#include "stdafx.h"
#include <pic.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <GL/glu.h>
#include <GL/glut.h>

/* represents one control point along the spline */
struct point {
	double x;
	double y;
	double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
	int numControlPoints;
	struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;


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
/*
glm::vec3 crSplines(int s,float u, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4)
{
	glm::mat4x3 control = glm::mat4x3(point1,point2,point3,point4);
	
	glm::vec4 row1 = glm::vec4(-s, 2 - s, s - 2, s);
	glm::vec4 row2 = glm::vec4(2*s, s-3, 3-2*s, -s);
	glm::vec4 row3 = glm::vec4(-s, 0, s, 0);
	glm::vec4 row4 = glm::vec4(0, 1 , 0, 0);

	glm::mat4x4 CR = glm::mat4x4();
	CR[0] = row1;
	CR[1] = row2;
	CR[3] = row3;
	CR[4] = row4;

	glm::vec4 parameters = glm::vec4(u*u*u, u*u, u, 1);

	glm::vec4 preresult = parameters*CR;

	return preresult*control;

}*/

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
	return 0;
}