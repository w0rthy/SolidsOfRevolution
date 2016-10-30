#include <GL\freeglut.h>
#include <cstdio>
#include <cmath>

#pragma comment(lib, "freeglut.lib")

int Width = 640;
int Height = 640;
bool DRAWING = false;
bool DRAWMODE = true;
//0 = OUTLINE 1 = SOLID
int DRAWSTYLE = false;
int timeoffset = 0;
int VERTEXLIM = 50;
float rotamt = 1.0f;
float rotate_x = 0, rotate_y = 0, zoom = 1.0f;

bool FOG = true;

struct vert {
	vert(){}

	vert(float X, float Y){
		x = X;
		y = Y;
	}

	float x, y;
};

vert* shapedata = nullptr;
int shapedatalen = 0;

struct vertexdata {
	vertexdata() {}

	vertexdata(int X, int Y){
		prev = nullptr;
		c = 0;
		x = X;
		y = Y;
	}
	vertexdata(vertexdata* a, int X, int Y) {
		x = X;
		y = Y;
		c = a->c+1;
		prev = a;
	}

	int c, x, y;
	vertexdata* prev;
};

vertexdata* vertdata = nullptr;

void delvertexdata(vertexdata* a){
	vertexdata* t = a;
	while(t->prev!=nullptr){
		vertexdata* tmp = t->prev;
		delete t;
		t = tmp;
	}
	delete t;

}

void compilevertexdata(){
	if(vertdata == nullptr)
		return;
	int c = vertdata->c;
	int verts = 0;
	float amt = (float)c/(float)VERTEXLIM;
	float rem = 0.0f;
	vertexdata* v = vertdata;

	if(shapedata != nullptr)
		delete[] shapedata;

	shapedata = new vert[VERTEXLIM];
	shapedatalen = VERTEXLIM;

	while(v != nullptr && verts < shapedatalen){
		int toskip = (int)amt + (int)rem;
		rem -= (float)(int)rem;
		rem += amt - (float)(int)amt;
		while(toskip > 0){
			toskip--;
			v = v->prev;
			if(v == nullptr)
				break;
		}
		if(v == nullptr)
			break;

		shapedata[verts] = vert(
			(float)v->x/(float)Width*2.0f-1.0f,
			(float)v->y/(float)Height*2.0f-1.0f);
		verts++;
	}

	delvertexdata(vertdata);
	vertdata = nullptr;

}

void drawShape(){

	if(shapedata == nullptr) return;
	glBegin(DRAWSTYLE?GL_LINE_STRIP:GL_POLYGON);
	for(int i = 0; i < shapedatalen; i++){
		glVertex2f(shapedata[i].x,shapedata[i].y);
	}
	glEnd();
}

void mainLoop(){
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	//glOrtho(-zoom,zoom,zoom,-zoom,-zoom,zoom);

	if(!DRAWMODE){
		glScalef(zoom,zoom,zoom);
		glRotatef(rotate_x,1.0,0,0);
		glRotatef(rotate_y,0,1.0,0);

		glColor3f(0.0f,1.0f,0.0f);
		glBegin(GL_LINES);
		glVertex2f(0,-1);
		glVertex2f(0,1);
		glEnd();


		float i = (float)(glutGet(GLUT_ELAPSED_TIME)-timeoffset)/50.0f;
		if(i>360) i = 360.0f;
			for(; i > 0; i-=rotamt){
				glRotatef(rotamt,0.0,1.0,0.0);
				//OUTLINE (LINES) OR FILL (POLYGON)
				float col = 1.0f-(float)i/360.0f;
				glColor3f(1.0f,col,col);
				drawShape();
			}
	}else{
		glColor3f(1.0f,1.0f,1.0f);
		if(!DRAWING)
			drawShape();
		else{
			if(vertdata != nullptr){
				vertexdata* v = vertdata;
				glBegin(DRAWSTYLE?GL_LINE_STRIP:GL_POLYGON);
				while(v->prev != nullptr){
					glVertex2f(
						(float)v->x/(float)Width*2.0f-1.0f,
						(float)v->y/(float)Height*2.0f-1.0f);
					v = v->prev;
				}
				glEnd();
			}
		}
		glColor3f(0.0f,1.0f,0.0f);
		glBegin(GL_LINES);
		glVertex2f(0,-1);
		glVertex2f(0,1);
		glEnd();
	}
	
	glFlush();
	glutSwapBuffers();
}

int prevx = 0, prevy = 0;
void passivemotion(int x, int y){
	prevx = x;
	prevy = y;
}

void motion(int x, int y){
	if(!DRAWMODE){
		rotate_x += ((float)y-(float)prevy)/5;
		rotate_y -= ((float)x-(float)prevx)/5;
	}else{
		if(DRAWING){
			if(vertdata == nullptr)
				vertdata = new vertexdata(x,y);
			vertdata = new vertexdata(vertdata,x,y);
		}
	}
	
	prevx = x;
	prevy = y;
}

void mousewheel(int wheel, int direction, int x, int y){
	zoom += (float)direction/15.0f;
}

void mouse(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON && DRAWMODE){
		if(state == GLUT_DOWN){
			DRAWING = true;
		}else{
			DRAWING = false;
			compilevertexdata();
		}
	}
	else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && shapedata != nullptr){
		DRAWMODE = !DRAWMODE;
		timeoffset = glutGet(GLUT_ELAPSED_TIME)-50;
		rotate_x = 0.0f;
		rotate_y = 0.0f;
		zoom = 1.0f;
	}
	else if(button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN){
		DRAWSTYLE = !DRAWSTYLE;
	}
}

void dofog(){
	FOG?glEnable(GL_FOG):glDisable(GL_FOG);
}

void key(unsigned char key, int x, int y){
	if(key == 'f'){
		FOG = !FOG;
		dofog();
	}
	if(key == 't'){
		VERTEXLIM += 10;
		printf("VERTEX LIMIT - %i\n", VERTEXLIM);
	}
	if(key == 'g'){
		VERTEXLIM -= 10;
		printf("VERTEX LIMIT - %i\n", VERTEXLIM);
	}
	if(VERTEXLIM <= 10) VERTEXLIM = 10;
	if(key == 'y'){
		rotamt += 0.1f;
		printf("ROT AMT - %f\n", rotamt);
	}
	if(key == 'h'){
		rotamt -= 0.1f;
		printf("ROT AMT - %f\n", rotamt);
	}
	if(rotamt<=0.1f) rotamt = 0.1f;
}

void reshape(GLsizei width, GLsizei height){
	Width = width;
	Height = height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0,0,width,height);
	glOrtho(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
}

void idle(){
	glutPostRedisplay();
}

int main(int argc, char** argv){
	FreeConsole();

	//GL and GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(640,640);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Solids of Revolution");

	glutDisplayFunc(mainLoop);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passivemotion);
	glutMouseWheelFunc(mousewheel);
	glutMouseFunc(mouse);
	glutKeyboardFunc(key);

	//BACKGROUND COLOR
	glClearColor(0.0f,0.0f,0.0f,1.0f);

	//FLAGGING
	glDisable(GL_LIGHTING);

	//ANTI-ALIASING
	
	glEnable(GL_FOG);
	glHint(GL_FOG_HINT,GL_FASTEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT,GL_FASTEST);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT,GL_FASTEST);
	//glEnable(GL_POINT_SMOOTH);
	//glHint(GL_POINT_SMOOTH_HINT,GL_FASTEST);

	glutMainLoop();

	return 0;
}