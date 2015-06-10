#include <iostream>

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "Point.h"
#include "Line.h"
#include <gl/glut.h>
using namespace std;
#define WIDTH 800
#define HEIGHT 800

// The time in milliseconds between timer ticks
#define TIMERMSECS 33

// rotation rate in degrees per second
#define ROTRATE 45.0f

#define gravity -9.81f
//collision bool
bool collided = false;

// Global variables for measuring time (in milli-seconds)
int startTime;
int prevTime;

static GLfloat rot = 0.0f;

//  ------ mouse stuff
int lmbDown = 0;
int rmbDown = 0;
float oldPos[2], newPos[2];
CPoint tmpPts[2];
int hover = -1, dragging = -1;
int hoveringOver = -1; int hoveringStartingVertice = -1; 
bool mouseOnBall = false; bool mouseOnBall2 = false;
// ----- vertex and line list
vector<CPoint> vPts;

int beginDrawing = 0; 

// ----- keyboard stuff
bool beginSimulation = false;
float clicked = false;
// ----- SHAPE OBJECTS -----
	struct object {
		float weight;
		float xVelocity;
		float yVelocity;
		float zVelocity;
		float initXVelocity;
		float initYVelocity;
		float firstX;
		float firstY;
		float firstZ;
		float initX;
		float initY;
		float initZ;
		float x;
		float y;
		float z;
		float radius;
		float deltaY;
		//float acceleration;
		//float impulse;
		float beginMotionTime;
		bool inMotion;
		float xDistanceFromNail;
		float yDistanceFromNail;
		float width, height;
		float xBeginMotionTime;
	} sphere, spherebox, sphere2, spherebox2;
//vector<object> objects;
	object objects[2];
	// ----- Function Prototypes -----
static void init(int w, int h);
static void reshape(GLsizei w, GLsizei h);
static void animate(int value);
static void render();
static void key(unsigned char k, int x, int y);
static void setupLights();
static void initShapes();
static void mouse(int button, int state, int x, int y);
static void motion(int x, int y);
static void pmotion(int x, int y);
static void win2world(float* worldpos, int x, int y);
// ---- Function Implementations -----


int main(int argc, char** argv)
{
    glutInit(&argc, argv);

	// Set up the GLUT window
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(500, 300);
	glutInitWindowSize (WIDTH, HEIGHT);
    glutCreateWindow (argv[0]);

	// Set up the callbacks
	init(WIDTH, HEIGHT);
	glutDisplayFunc(render);
	glutKeyboardFunc(key);
	glutMouseFunc( mouse );
	glutMotionFunc( motion );
	glutPassiveMotionFunc( pmotion );
    glutReshapeFunc(reshape);
	glutPostRedisplay();

	setupLights();

	initShapes();

	// Start the timer when user presses 1
    glutTimerFunc(TIMERMSECS, animate, 0);
	// Initialize the time variables
	startTime = glutGet(GLUT_ELAPSED_TIME);
	prevTime = startTime;
	
	// Start the main loop
    glutMainLoop();

    return 0;
}

static void init(int w, int h)
{
	// Set up the OpenGL state
	// ##### REPLACE WITH YOUR OWN OPENGL INIT CODE HERE #####
    glClearColor(1.0, 1.0, 1.0, 0.0);
	reshape(w, h);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// ##### END OF INIT CODE #####
}

static void reshape(GLsizei w, GLsizei h)
{
	// Respond to a window resize event

	// ##### REPLACE WITH YOUR OWN RESHAPE CODE #####
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	// Calculate the aspect ratio of the screen, and set up the
	// projection matrix (i.e., update the camera)
	gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void applyGravity(object& shape, float timeInMotion) {
	
	//Y = V_i * t + (t^2*a*1/2) + Y_i
	shape.y = shape.initY + (timeInMotion * timeInMotion * gravity * .5) + (timeInMotion * shape.initYVelocity);
	//V = V_i + a*t
	shape.yVelocity = shape.initYVelocity + gravity * timeInMotion;
}
//Equation to calculate time it takes for object to hit the ground
//0 = 1/2 gt^2 + v_0 * t - changeY
static float quadraticEquation(float a, float b, float c) {

	float ac4 = 4*a*c;

	if (4*a*c > b*b) {
		printf("a: %f, b: %f, c: %f \n", a,b,c);
		ac4 *= -1;
	}
	float root1 = (-b + sqrt(b*b - ac4))  / (2*a);
	float root2 = (-b - sqrt(b*b - ac4))  / (2*a);
	if (root1 > root2) {
		return root1;
	}
	else {
		return root2;
	}
}
static float initYVelocityToReachPont(float sphereY, float vPtsY) {
	//v^2 = v_i ^2 + 2a deltaY
	float deltaY = vPtsY-sphereY;
	float initYVelocity = sqrt(-2*gravity*deltaY);
	return initYVelocity;
}

float currTime = 0;
float timeSincePrevFrame = 0;
float elapsedTime = 0;
float timeBeforeStarting = 0;
float timeToLand = 0;
int currentVertex = 1;
bool clicked1 = true;
bool newVertex = false;
static void animate(int value)
{
	//printf("%f\n", elapsedTime);
    glutTimerFunc(TIMERMSECS, animate, 0);
	//THIS IF STATEMENT IS RUN UNTIL USER BEGINS THE SIMULATION (AFTER PLOTTING POINTS)
	if (beginSimulation == false) {
		timeBeforeStarting = glutGet(GLUT_ELAPSED_TIME);
		//find time until collision with ground
		float a,b,c;
		if (clicked == false) return;
		//VERTEX IS ABOVE THE SPHERE
			if (objects[0].y < vPts[currentVertex].y) {
				//CALCULATE V_INIT FOR BALL TO HAVE Y-VEL. OF 0 AT POINT
				objects[0].initYVelocity = initYVelocityToReachPont(objects[0].y, vPts[currentVertex].y);
				
				a = -4.905;
				b = objects[0].initYVelocity;
				c = vPts[currentVertex].y - objects[0].initY;
			}
			//VERTEX IS BELOW THE objects[0]
			else {
				a = -4.905;
				b = objects[0].initYVelocity;
				c = objects[0].initY - vPts[currentVertex].y;
			}
			//CALCULATE TIME IT TAKES TO BE 0 VELOCITY AT VERTEX HEIGHT Y
			timeToLand = quadraticEquation(a,b,c);
			
			//IF THE VERTEX IS ON THE LEFT
			if(objects[0].x > vPts[currentVertex].x) {
				objects[0].xDistanceFromNail = objects[0].x-vPts[currentVertex].x;
				objects[0].xDistanceFromNail*=-1;
			} //VERTEX ON THE RIGHT
			else {objects[0].xDistanceFromNail = vPts[1].x-objects[0].x;}
			
			//X VELOCITY NEEDED TO REACH THAT VERTEX IN THE CALCULATED TIME
			objects[0].initXVelocity = 2*objects[0].xDistanceFromNail/timeToLand;
			
			if (objects[0].y>vPts[currentVertex].y) {objects[0].initXVelocity*=.5;}
			objects[0].xVelocity = objects[0].initXVelocity;
			
			//CALCULATE THIS ONLY ONCE
			
			clicked = false;
		return;
	}
	else { //----SIMULATION HAS BEGUN\
		//Calculate from 1 vertex to the next: initY, initX, yvelocity, xvelocity
			//REACHED VERTEX, NOW TO THE NEXT VERTEX = CALCULATE NEW Y VELOCITY
			if (currentVertex > 1 && clicked1 == true && vPts.size() != currentVertex && newVertex == true) {
				float a,b,c;

				if (objects[0].y < vPts[currentVertex].y) {
					//CALCULATE V_INIT FOR BALL TO HAVE Y-VEL. OF 0 AT POINT
					objects[0].initYVelocity = initYVelocityToReachPont(objects[0].y, vPts[currentVertex].y);
				
					a = -4.905;
					b = objects[0].initYVelocity;
					
					c = vPts[currentVertex].y - objects[0].initY;
				}
				//VERTEX IS BELOW 
				else {
					a = -4.905;
					b = objects[0].initYVelocity;
					c = objects[0].initY - vPts[currentVertex].y;
				}


				//CALCULATE TIME IT TAKES TO BE 0 VELOCITY AT VERTEX HEIGHT Y
				timeToLand = quadraticEquation(a,b,c);
				
				//IF THE VERTEX IS ON THE LEFT
				if(objects[0].x > vPts[currentVertex].x) {
					objects[0].xDistanceFromNail = objects[0].x-vPts[currentVertex].x;
					objects[0].xDistanceFromNail*=-1;
				} //VERTEX ON THE RIGHT
				else {objects[0].xDistanceFromNail = vPts[currentVertex].x-objects[0].x;}
				
				//X VELOCITY NEEDED TO REACH THAT VERTEX IN THE CALCULATED TIME
				objects[0].initXVelocity = 2*objects[0].xDistanceFromNail/timeToLand;
				if (objects[0].y>vPts[currentVertex].y) {objects[0].initXVelocity*=.5;}
				//CALCULATE THIS ONLY ONCE
				
				
		}

		// ---------------------------------------- TIME-----------------------------	
		currTime = glutGet(GLUT_ELAPSED_TIME);
		timeSincePrevFrame = currTime - prevTime;
		elapsedTime = (currTime - startTime - timeBeforeStarting)/1000;
		collided = false;
		if (elapsedTime < 0) elapsedTime = 0;
		float timeInMotion;
		for (int i = 0; i < 2; i++) {
			float timex = elapsedTime - objects[1].xBeginMotionTime;
			timeInMotion = elapsedTime - objects[i].beginMotionTime;
			
			//printf("%f\n", timeInMotion);
			if (timeInMotion < 0) {
				timeInMotion = 0;
			}
			//-----------------------------COLLISION BETWEEN OBJECTS--------------------
				//collision between bounding boxes
			if (i==0 && collided == false)
					if(abs(objects[0].x-objects[1].x)*2 < objects[0].radius+objects[1].radius &&
						abs(objects[0].y-objects[1].y)*2 < objects[0].radius+objects[1].radius
						) {
						//swap velocities
							
							float initXVelocity = objects[1].initXVelocity;
							float time = objects[1].beginMotionTime;
							float initX = objects[1].initX;
							
							objects[1].initXVelocity = objects[0].initXVelocity/2;
							objects[1].xBeginMotionTime = elapsedTime;
							objects[1].initX = objects[1].x;
		
							objects[0].initXVelocity *= -.5;
						
							objects[0].initX = objects[0].x;
							collided = true;

					}

			



			// -------------------------------- PHYSICS MOTION ------------------------
			if (objects[i].inMotion == false) continue;
			
			

			//apply horizontal x velocity while in motion
			objects[i].x = objects[i].initX + objects[i].initXVelocity * (timeInMotion);
			//objects[i].xVelocity = objects[i].initXVelocity;
			if(collided && i == 1) {
				objects[i].x = objects[i].initX + objects[i].initXVelocity * (timex);
			}

			//---------------------------------GRAVITY----------------------------------
			if (objects[i].y > 0) {
				applyGravity(objects[i], timeInMotion);
			}
			//	-------------------------COLLISION WITH GROUND--------------------------------
			else { 
				
				//V_f = V_i^2 + 2aDeltaY , delta y is just distance from initY to floor (0)
				float finalVelocitySquared = 
					(objects[i].initYVelocity * objects[i].initYVelocity) + 2*gravity*objects[i].initY;
				
				//Damping forces
				//Need to double square root to make positive
			
				objects[i].yVelocity = .75 * sqrt(sqrt(finalVelocitySquared * finalVelocitySquared));
				
				objects[i].xVelocity *= .9;

				while (objects[i].yVelocity > objects[i].initYVelocity && objects[i].initYVelocity != 0){
					objects[i].yVelocity *= .75;
				}
				objects[i].initYVelocity = objects[i].yVelocity;
				//printf("%f\n", objects[i].initY);

				if (objects[i].yVelocity < .003){
					objects[i].yVelocity = 0; objects[i].y = 0; objects[i].inMotion = false;
					objects[i].xVelocity = 0;
				}
				objects[i].y = 0.01;
				objects[i].initY = objects[i].y;
				objects[i].initX = objects[i].x;
				objects[i].beginMotionTime = elapsedTime;
			}

		}
	}


	newVertex = false;
	if (!collided) {
	//---------------------------------BALL REACHED THE VERTEX--------------------------------
	if (objects[0].inMotion == true && vPts.size() == 2 && currentVertex < vPts.size()) {
		//NEED TO FIX WHEN ONLY 1 VERTEX
		if (vPts[currentVertex].x>=objects[0].initX) { 
			if (objects[0].x>=vPts[currentVertex].x && vPts[currentVertex].y > objects[0].initY) {
					objects[0].initX = objects[0].x;
					objects[0].initY = objects[0].y;
					objects[0].initYVelocity = 0;
					objects[0].yVelocity = 0;
					objects[0].beginMotionTime = elapsedTime;
			}
		}else if (vPts[currentVertex].x<=objects[0].initX) {
			if(objects[0].x<=vPts[currentVertex].x && vPts[currentVertex].y > objects[0].initY) {
					objects[0].initX = objects[0].x;
					objects[0].initY = objects[0].y;
					objects[0].initYVelocity = 0;
					objects[0].yVelocity = 0;
					objects[0].beginMotionTime = elapsedTime;
			
			}
		}
	}
	if (objects[0].inMotion == true && vPts.size() > 2 && currentVertex < vPts.size()) {
		//IF VPTS IS ON THE RIGHT, WHEN objects[0] PASSES IT, KEEP IT STILL
		if (vPts[currentVertex].x>=objects[0].initX) { 
			if (objects[0].x>=vPts[currentVertex].x) {
						
				//objects[0].x = vPts[currentVertex].x;
				//objects[0].y = vPts[currentVertex].y;
				

				if (currentVertex + 1 != vPts.size()) {
					objects[0].initX = objects[0].x;
					objects[0].initY = objects[0].y;
						
					objects[0].yVelocity=0;
					objects[0].initXVelocity = objects[0].xVelocity;
			
					objects[0].initYVelocity = 0;
					objects[0].beginMotionTime = elapsedTime;
				}
				
				if (currentVertex + 1 != vPts.size()) {
				newVertex = true;
				//INCREMENT TARGET VERTEX
				currentVertex += 1;
				//objects[0].inMotion = false;

				}
			}
		} //IF VPTS ON LEFT, WHEN objects[0] PASS, KEEP STILL
		else if (vPts[currentVertex].x<=objects[0].initX) {
			if(objects[0].x<=vPts[currentVertex].x) {
				
				//objects[0].x = vPts[currentVertex].x;
				//objects[0].y = vPts[currentVertex].y;
				
				if (currentVertex + 1 != vPts.size() || vPts.size() == 2) {
					objects[0].initX = objects[0].x;
					objects[0].initY = objects[0].y;
					//objects[0].xVelocity=0;
					objects[0].yVelocity=0;
					objects[0].initXVelocity = objects[0].xVelocity;
					objects[0].initYVelocity = 0;
					objects[0].beginMotionTime = elapsedTime;
				}
				if (currentVertex + 1 != vPts.size()) {
				newVertex = true;
				//INCREMENT TARGET VERTEX
				currentVertex += 1;
				//objects[0].inMotion = false;			}
				}
			}
		}
	
	}
	}
	// Rotate the triangle
	rot = (ROTRATE / 1000) * elapsedTime;

	// Force a redisplay to render the new image
	glutPostRedisplay();

	prevTime = currTime;
}
GLUquadricObj *qobj = gluNewQuadric();
static void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glLoadIdentity();

	
	
	//glColor3f(1,.2,0);
	//glTranslatef(0.0f,2.3f,-12.0f);
	//glRotatef(60,0,1,0);

	//gluCylinder(qobj, 0,.5,1,15,5); //draw cone
	//glTranslatef(0,0,0);
	//gluDisk(qobj, 0,.5,20,4);



	//draw cube as ground
	glPushMatrix();
	glColor3f(1,1,1);
	glScalef(8,1,1);
	glTranslatef(.0f,-1.3f,-12.0f);
	glutSolidCube(2.0);
	glPopMatrix();

	//draw sphere 1
	glPushMatrix();
	if (mouseOnBall == true) {
		glColor3f(0,1,1);
	}
	else {
		glColor3f(0,.5,0);
	}
	glTranslatef(objects[0].x, objects[0].y, -12.0f);
	
	glutWireCube(objects[0].radius * 2);
	gluSphere(qobj, objects[0].radius, 15, 10); 
	
	glPopMatrix();

	//draw sphere 2
	glPushMatrix();
	if (mouseOnBall2 == true) {
		glColor3f(0,1,1);
	} else {
		glColor3f(1,0,0);}
	glTranslatef(objects[1].x, objects[1].y, -12.0f);
	glutWireCube(objects[1].radius * 2);
	gluSphere(qobj, objects[1].radius, 15, 10); 
	
	glPopMatrix();


	glPushMatrix();
	//draw all vertices
	for (int i=0; i<vPts.size(); i++)
	{		
		
		// set anchor color according to its state
		//red color for dragging
		if (i == dragging)
		{
			glColor3f(1.0, 0.0, 0.0);
		}
		//purple when mouse on top of the vertexs
		else if (i == hover)
		{
			glColor3f(0.5f, 0.0f, 1.0f);
		}
		//default vertex color (green)
		else
		{
			glColor3f(0.0, 1.0, 0.0);
		}

			glPointSize(13);
				glBegin(GL_POINTS);
					
					glVertex3f(objects[0].firstX, objects[0].firstY, -12);
					glVertex3f(vPts[i].x, vPts[i].y, -12);
					
					//glVertex3f(sphere.xVelocity, sphere.yVelocity, -12);
					
				glEnd();

	}
	glPopMatrix();

	glPushMatrix();
	//DRAWING LINES BETWEEN VERTICES
	if (vPts.size() ==  1) {
		glColor3f(1.0, 0.0, 0.0);
				glBegin(GL_LINES);
				glVertex3f(sphere.firstX, sphere.firstY, -12);
				glEnd();
		
	}
	
	if (vPts.size() >  1) {
		glColor3f(1.0, 0.0, 0.0);
				glBegin(GL_LINES);
				for (int i = 0; i < vPts.size()-1; i++) {
					glVertex3f(vPts[i].x,vPts[i].y, -12);
					glVertex3f(vPts[i+1].x,vPts[i+1].y, -12);
				}
				glEnd();
		
	}
	glPopMatrix();

	
    glutSwapBuffers();
    glFlush();
}

static void key(unsigned char k, int x, int y)
{
	switch (k) {
	case 27:  /* Escape */
		exit(0); // quit
		break;
	case '1':
		beginSimulation = true;
		break;
	case '2':
		initShapes();
		currTime = 0;
		timeSincePrevFrame = 0;
		elapsedTime = 0;
		timeBeforeStarting = 0;
		timeToLand = 0;
		currentVertex = 1;
		sphere.beginMotionTime = glutGet(GLUT_ELAPSED_TIME);
		break;
	default:
		return;
	}

	// Force a redraw of the screen in order to update the display
	glutPostRedisplay();
}

static void setupLights() {
	GLfloat lightOnePosition[4] = {0, 0, 1, 0};
	GLfloat lightColor[] = {1, 1, 1, 1};	// light color
	GLfloat lightTwoPosition[4] = {0, 0, -1, 0};
	GLfloat globalAmb[] = {.1, .1, .1, 1};	// ambience lighting
	glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT2, GL_POSITION, lightTwoPosition);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

	glClearColor(1.0,1.0,1.0,1.0);

	// enable lighting & the lights specified
	// OpenGL supports GL_LIGHT0 ~ GL_LIGHT7
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_CULL_FACE);	// enable face culling; faces with clockwise vertices are treated							// as back faces and removed directly
	glEnable(GL_DEPTH_TEST);	// enable depth test for hidden surface removal
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_FLAT);
}
static void initShapes() {
	sphere.initX = -2;
	sphere.initY = 3;
	sphere.initXVelocity = 0;
	sphere.initYVelocity = 0;
	sphere.yVelocity = sphere.initYVelocity;
	sphere.xVelocity = sphere.initXVelocity;
	sphere.x = sphere.initX;
	sphere.y = sphere.initY;
	sphere.beginMotionTime = 0;
	sphere.xBeginMotionTime = 0;
	sphere.inMotion = true;
	sphere.firstX = sphere.x;
	sphere.firstY = sphere.y;
	sphere.radius = .3;
	CPoint point;
	point.x = sphere.firstX;
	point.y = sphere.firstY;
	vPts.clear();
	vPts.push_back(point);

	spherebox = sphere;
	spherebox.height = sphere.radius;
	spherebox.width = sphere.radius;


	sphere2.initX = 2;
	sphere2.initY = 3;
	sphere2.initXVelocity = 0;
	sphere2.initYVelocity = 0;
	sphere2.yVelocity = sphere2.initYVelocity;
	sphere2.xVelocity = sphere2.initXVelocity;
	sphere2.x = sphere2.initX;
	sphere2.y = sphere2.initY;
	sphere2.beginMotionTime = 0;
	sphere2.beginMotionTime = 0;
	sphere2.inMotion = true;
	sphere2.firstX = sphere2.x;
	sphere2.firstY = sphere2.y;
	sphere2.radius = .3;
	spherebox2 = sphere2;
	spherebox2.height = sphere.radius;
	spherebox2.width = sphere.radius;

	/*
	CPoint point;
	point.x = sphere.firstX;
	point.y = sphere.firstY;
	vPts.clear();
	vPts.push_back(point);*/

	objects[0] = sphere;
	objects[1] = sphere2;

}

// windows position -> world coordinate conversion
static void win2world(float* worldpos, int x, int y)
{

	GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
    
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
 
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
    // 0.1 is for giving a depth offset from the eye position
    gluUnProject( winX, winY, 0.1, modelview, projection, viewport, &posX, &posY, &posZ);

	worldpos[0] = (100*posX) / 4.6 * 5;
	worldpos[1] = (100*posY) / 4.6 * 5;
	
	
}

// mouse click event handler
static void mouse(int button, int state, int x, int y)
{
	
	// left button down - plot vertices
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		lmbDown = 1;
		win2world(oldPos, x, y);
		tmpPts[0].x = oldPos[0];
		tmpPts[0].y = oldPos[1];
		tmpPts[1].x = oldPos[0];
		tmpPts[1].y = oldPos[1];
		
	}
	//release left mouse click and finalize vertice
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		if (vPts[vPts.size()-1].x == tmpPts[0].x ||
			vPts[vPts.size()-1].y == tmpPts[0].y) {
			return;

		}
		
		lmbDown = 0;
		win2world(newPos, x, y);
	
		vPts.push_back( tmpPts[0] );
		glutPostRedisplay();
		clicked = true;
	}
	// changed to end of input process aka connect the vertices
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
	{
		
			rmbDown = 1;
			
			win2world(oldPos, x, y);
			// set the vertex to edit
			dragging = hover;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		rmbDown = 0;
		win2world(newPos, x, y);
		// end dragging
		dragging = -1;
	}
	

}

// mouse move event handler
static void motion(int x, int y)
{
	
	float tempPos[2];
	win2world(tempPos, x, y);
	//mouseOnBall = false;		
	
	// RMB down + move = RMB drag
	// redraw the line for preview
	if (lmbDown == 1)
	{
		tmpPts[1].x = tempPos[0];
		tmpPts[1].y = tempPos[1];
	
	}
	
	else if (rmbDown == 1)
	{
		// right mouse button down - moving mode
		if (dragging != -1)
		{	
			vPts[dragging].x = tempPos[0];
			vPts[dragging].y = tempPos[1];
			clicked = true;	
		}
		if (mouseOnBall == true || mouseOnBall2 == true){int i = 0;
			if (mouseOnBall) {
				i = 0;
				spherebox.x = tempPos[0];
				spherebox.y = tempPos[1];
			}
			else if (mouseOnBall2){
				i = 1; 
				spherebox2.x = tempPos[0];
				spherebox2.y = tempPos[1];
			}	
			objects[i].x = tempPos[0];
			objects[i].y = tempPos[1];
			objects[i].firstY = objects[i].y;
			objects[i].firstX = objects[i].x;
			objects[i].initX = objects[i].x;
			objects[i].initY = objects[i].y;
			
			clicked = false;
		}

		
	}
	glutPostRedisplay();
}

// mouse passive motion event handler
static void pmotion( int x, int y )
{
	// no mouse button is down: check if mouse is over an anchor
	float tempPos[2];
	bool bFound = false;

	win2world(tempPos, x, y);
	for (int i=0; i<vPts.size(); i++)
	{
		if ( (tempPos[0]-vPts[i].x < 30./WIDTH
			&& tempPos[0]-vPts[i].x>-30./WIDTH)
			&& (tempPos[1]-vPts[i].y<30./HEIGHT
			&& tempPos[1]-vPts[i].y>-30./HEIGHT) )
		{
			hover = i;
			bFound = true;
			break;
		}
		if (i == 0){	
			if (tempPos[0] >= spherebox.x-spherebox.width  && tempPos[0] <= spherebox.x+spherebox.width &&
					tempPos[1] >= spherebox.y-spherebox.height  && tempPos[1] <= spherebox.y+spherebox.height){
				mouseOnBall = true;
				hover = 0;
				
				bFound = true;
				break;
			} else if (tempPos[0] >= spherebox2.x-spherebox.width  && tempPos[0] <= spherebox2.x+spherebox.width &&
					tempPos[1] >= spherebox2.y-spherebox.height  && tempPos[1] <= spherebox2.y+spherebox.height){
				mouseOnBall2 = true;
				
				bFound = true;
				break;
			}
			else {	
				mouseOnBall = false;
				mouseOnBall2 = false;
			}

		}
	}
	if (!bFound)
		hover = -1;

	glutPostRedisplay();
}