// JUAN CARLOS GONZALEZ GUERRA


#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Cylinder.h"
#include "Utils.h"
#include "Mat4.h"
#include "Transforms.h"
#include <stdio.h>
#include <math.h>

#define M_PI 3.14159265358979323846
#define RESET 0xFFFF
#define new_max(x,y) ((x) >= (y)) ? (x) : (y)
#define new_min(x,y) ((x) <= (y)) ? (x) : (y)
#define radDiff (topRadius - bottomRadius)

typedef enum { IDLE, LEFT, RIGHT, UP, DOWN, FRONT, BACK } MOTION_TYPE;
typedef float vec3[3];

static GLuint vertexPositionLoc,  vertexNormalLoc, projectionMatrixLoc,  viewMatrixLoc;
static GLuint vertexColorLoc, vertexPositionLoc, vertexNormalLoc;
static GLuint cameraPositionLoc;
static GLuint ambientLightLoc, diffuseLightLoc, lightPositionLoc, materialALoc, materialDLoc, materialSLoc, exponentLoc;
static Mat4   projectionMatrix, viewMatrix;
static GLuint programId, modelMatrixLoc;
static Mat4   modelMatrix;

static MOTION_TYPE motionType 		= 0;

static float cameraSpeed     		= 0.1;
static float cameraX        		= 0;
static float cameraZ         		= 0;
static float cameraY		 		= 0;
static float cameraAngle     		= 0.0;
const static float rotationSpeed 	= -0.1;
static float rotationCounter 		= 0;
const static float width 			= 1;
const static float height 			= width * 0.8;

float currentXWall;
float currentZWall;

static float ambientLight[]  	= {1.0, 1.0, 1.0};
static float materialA[]     	= {1.0, 1.0, 1.0};
static float diffuseLight[]  	= {0.0, 0.0, 0.0};
static float lightPosition[] 	= {0.0, 0.0, 0.0};
static float materialD[]     	= {0.6, 0.6, 0.6};
static float materialS[]		= {0.0, 0.0, 0.0};
static float exponent			= 20;

float length = 40, bottomRadius = 4, topRadius = 1;
int slices = 100, stacks = 100;
vec3 topColor 		= { 0.2, 0.5, 1.0 };
vec3 bottomColor 	= { 0.4, 0.1, 0.3 };

Cylinder cylinder;

static void initShaders()
{
	cylinder = cylinderCreate(length, bottomRadius, topRadius, slices, stacks, bottomColor, topColor);

	GLuint vShader = compileShader("shaders/gouraud.vsh", GL_VERTEX_SHADER);
	if(!shaderCompiled(vShader)) return;
	GLuint fShader = compileShader("shaders/color.fsh", GL_FRAGMENT_SHADER);
	if(!shaderCompiled(fShader)) return;

	programId = glCreateProgram();
	glAttachShader(programId, vShader);
	glAttachShader(programId, fShader);
	glLinkProgram(programId);

	vertexPositionLoc   = glGetAttribLocation(programId, "vertexPosition");
	vertexNormalLoc     = glGetAttribLocation(programId, "vertexNormal");
	vertexColorLoc    	= glGetAttribLocation(programId, "vertexColor");

	modelMatrixLoc      = glGetUniformLocation(programId, "modelMatrix");
	viewMatrixLoc       = glGetUniformLocation(programId, "viewMatrix");
	projectionMatrixLoc = glGetUniformLocation(programId, "projMatrix");

	ambientLightLoc     = glGetUniformLocation(programId, "ambientLight");
	diffuseLightLoc     = glGetUniformLocation(programId, "diffuseLight");
	lightPositionLoc    = glGetUniformLocation(programId, "lightPosition");
	materialALoc        = glGetUniformLocation(programId, "materialA");
	materialDLoc        = glGetUniformLocation(programId, "materialD");
	materialSLoc        = glGetUniformLocation(programId, "materialS");
	exponentLoc 		= glGetUniformLocation(programId, "exponent");
	cameraPositionLoc   = glGetUniformLocation(programId, "camera");

	//BINDING CYLINDER
	cylinderBind(cylinder, vertexPositionLoc, vertexColorLoc, vertexNormalLoc);
}

static void initLights()
{
	glUseProgram(programId);
	glUniform3fv(ambientLightLoc,  1, ambientLight);
	glUniform3fv(diffuseLightLoc,  1, diffuseLight);
	glUniform3fv(lightPositionLoc, 1, lightPosition);
	glUniform3fv(materialALoc,     1, materialA);
	glUniform3fv(materialDLoc,     1, materialD);
	glUniform3fv(materialSLoc,     1, materialS);
	glUniform1f(exponentLoc, exponent);
}

static float calculateXWall(float zPos)
{
	return fabs(bottomRadius + (radDiff * ((length / 2 - zPos)/length)));
};

static int isInside(float x, float z)
{
	if(cameraZ  < -length / 2 || cameraZ  > length / 2)
	{
		return 0;
	}
	if(cameraX + cameraSpeed <= -currentXWall) { return 0; }
	if(cameraX - cameraSpeed >= currentXWall)  { return 0; }

	return 1;
}

static int canMove(MOTION_TYPE m)
{
	currentXWall = calculateXWall(cameraZ);
	if(isInside(cameraX, cameraZ) == 0) { return 1; }
	if(m == LEFT)
	{
		if(cameraX <= -currentXWall) { return 1; }
		if(cameraX <= currentXWall)
		{
			if(cameraX - cameraSpeed * 2 >= -currentXWall) { return 1; }
		}
		if(cameraX - cameraSpeed * 2 >= currentXWall) { return 1; }

		return 0;
	}
	else if(m == RIGHT)
	{
		if(cameraX >= currentXWall) { return 1; }
		if(cameraX >= -currentXWall)
		{
			if(cameraX + cameraSpeed * 2 <= currentXWall) { return 1; }
		}
		if(cameraX + cameraSpeed * 2 <= -currentXWall) { return 1; }

		return 0;
	}
	else if(m == FRONT)
	{
		if(cameraX < -currentXWall || cameraX > currentXWall) { return 1; }
		if(topRadius >= bottomRadius) //FAR RADIUS BIGGER
		{
			if(cameraX < 0) //IF LEFT SIDE
			{
				if(cameraX <= -calculateXWall(cameraZ - cameraSpeed) + cameraSpeed) { return 0; }
			}
			else			//IF RIGHT SIDE
			{
				if(cameraX >= calculateXWall(cameraZ - cameraSpeed) - cameraSpeed) { return 0; }
			}
		}
		else			//CLOSER RADIUS BIGGER
		{
			if(cameraX < 0)	//IF LEFT SIDE
			{
				if(cameraX <= -calculateXWall(cameraZ - cameraSpeed) + cameraSpeed) { return 0; }
			}
			else			//IF RIGHT SIDE
			{
				if(cameraX >= calculateXWall(cameraZ - cameraSpeed) - cameraSpeed) { return 0; }
			}
		}

		return 1;
	}
	else if(m == BACK)
	{
		if(topRadius >= bottomRadius) //FAR RADIUS BIGGER
		{
			if(cameraX < 0) //IF LEFT SIDE
			{
				if(cameraX <= -calculateXWall(cameraZ + cameraSpeed)) { return 0; }
			}
			else			//IF RIGHT SIDE
			{
				if(cameraX >= calculateXWall(cameraZ + cameraSpeed)) { return 0; }
			}
		}
		else			//CLOSER RADIUS BIGGER
		{
			if(cameraX < 0)	//IF LEFT SIDE
			{
				if(cameraX <= -calculateXWall(cameraZ + cameraSpeed)) { return 0; }
			}
			else			//IF RIGHT SIDE
			{
				if(cameraX >= calculateXWall(cameraZ + cameraSpeed)) { return 0; }
			}
		}

		return 1;
	}

	return 0;
}

static void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(RESET);

	switch(motionType)
	{
  		case  LEFT  :
  			if(canMove(LEFT) == 1) { cameraX -= cameraSpeed; }
			break;

  		case  RIGHT :
  			if(canMove(RIGHT) == 1) { cameraX += cameraSpeed; }
  			break;

		case  FRONT :
  			if(canMove(FRONT) == 1) { cameraZ -= cameraSpeed; }
			break;

		case  BACK  :
  			if(canMove(BACK) == 1) { cameraZ += cameraSpeed; }
			break;

		case  UP    :break;
		case  DOWN  :break;
		case  IDLE  :  ;
	}

	// Envío de proyección, vista y posición de la cámara al programa 1 (cuarto, rombo)
	glUseProgram(programId);
	glUniformMatrix4fv(projectionMatrixLoc, 1, true, projectionMatrix.values);
	mIdentity(&viewMatrix);
	rotateY(&viewMatrix, -cameraAngle);
	translate(&viewMatrix, -cameraX, -cameraY, -cameraZ);
	glUniformMatrix4fv(viewMatrixLoc, 1, true, viewMatrix.values);
	glUniform3f(cameraPositionLoc, cameraX, cameraY, cameraZ);

	// Create cylinder
	scale(&modelMatrix, width, height, 1.0);
	rotateX(&modelMatrix, 90);
	rotateY(&modelMatrix, rotationCounter += rotationSpeed);
	glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
	cylinderDrawLines(cylinder);
	mIdentity(&modelMatrix);

	glutSwapBuffers();
}

static void reshapeFunc(int w, int h)
{
    if(h == 0) h = 1;
    glViewport(0, 0, w, h);
    float aspect = (float) w / h;
    setPerspective(&projectionMatrix, 45, aspect, -0.1, -500);
}

static void timerFunc(int id)
{
	glutTimerFunc(10, timerFunc, id);
	glutPostRedisplay();
}

static void specialKeyReleasedFunc(int key,int x, int y) {
	motionType = IDLE;
}

static void keyReleasedFunc(unsigned char key,int x, int y) {
	motionType = IDLE;
}

static void specialKeyPressedFunc(int key, int x, int y) {
	switch(key)
	{
		case 100:break;
		case 102:break;
		case 101:break;
		case 103:break;
	}
}

static void keyPressedFunc(unsigned char key, int x, int y)
{
/*	printf("CameraX = %.2f\n", cameraX);
	printf("CameraZ = %.2f\n", cameraZ);
	printf("Right wall = %.2f\n", currentXWall);
	printf("Left Wall = %.2f\n", -currentXWall);
	if(cameraX == currentXWall) { printf("Right Wall\n"); }
	if(cameraX == -currentXWall) { printf("Right Wall\n"); }*/

	switch(key)
	{
		case 'a':
		case 'A':
			motionType = LEFT;
			break;
		case 'd':
		case 'D':
			motionType = RIGHT;
			break;
		case 'w':
		case 'W':
			motionType = FRONT;
			break;
		case 's':
		case 'S':
			motionType = BACK;
			break;
		case 'e':
		case 'E': break;
		case 'q':
		case 'Q': break;
		case 27 : exit(0);
	}
 }

int main(int argc, char **argv)
{
	setbuf(stdout, NULL);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Cylinder");
    glutDisplayFunc(displayFunc);
    glutReshapeFunc(reshapeFunc);
    glutTimerFunc(10, timerFunc, 1);
    glutKeyboardFunc(keyPressedFunc);
    glutKeyboardUpFunc(keyReleasedFunc);
    glutSpecialFunc(specialKeyPressedFunc);
    glutSpecialUpFunc(specialKeyReleasedFunc);
    glewInit();
    glEnable(GL_DEPTH_TEST);

    initShaders();
    initLights();

    glClearColor(0.1, 0.1, 0.3, 1.0);
    glutMainLoop();

	return 0;
}
