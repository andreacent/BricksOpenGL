#include <iostream>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <math.h>

float radio = 0.3, px = 0.0, py = -8.0; // variables de ayuda para dibujar la pelota.

bool bloqueEspecial = true; // variable booleana para determinar si un bloque es especial o no
							// true si el bloque es especial, false si no.

int golpesEliminar = 0; // variable para saber con cuantos golpes se elimina la pelota.

bool bloqueBonus = false; // variable booleana para determinar si un bloque tiene bonus o no
						  // true si el bloque tiene bonus, false si no.

float bloques[7][5] = {}; //matriz para los bloques (creo que podria ser de int)
float especiales[5] = {}; //arreglo para los bloques especiales (creo que podria ser de int)
float bonus[6] = {};	  //arreglo para los bloques bonus (creo que podria ser de int)


using namespace std;

#define DEF_floorGridScale  1.0f
#define DEF_floorGridXSteps 10.0f
#define DEF_floorGridZSteps 10.0f

void ejesCoordenada(float w) {
  
  glLineWidth(w);
  glBegin(GL_LINES);
    glColor3f(1.0,0.0,0.0);
    glVertex2f(0,10);
    glVertex2f(0,-10);
    glColor3f(0.0,0.0,1.0);
    glVertex2f(10,0);
    glVertex2f(-10,0);
  glEnd();

  glLineWidth(w-1.0);
  int i;
  glColor3f(0.0,1.0,0.0);
  glBegin(GL_LINES);
    for(i = -10; i <=10; i++){
      if (i!=0) {   
        if ((i%2)==0){  
          glVertex2f(i,0.4);
          glVertex2f(i,-0.4);

          glVertex2f(0.4,i);
          glVertex2f(-0.4,i);
        }else{
          glVertex2f(i,0.2);
          glVertex2f(i,-0.2);

          glVertex2f(0.2,i);
          glVertex2f(-0.2,i);

        }
      }
    }
    
  glEnd();

  glLineWidth(1.0);
}

void dibujarPlataforma() {

	glPushMatrix();
		glColor3f(0.0,0.0,1.0);
		glBegin(GL_LINE_LOOP);
			glVertex2f(-2.0,-8.5);
			glVertex2f(2.0,-8.5);
			glVertex2f(2.0,-9.0);
			glVertex2f(-2.0,-9.0);
		glEnd();
	glPopMatrix();

}

void dibujarPelota() {

	glPushMatrix();
		glColor3f(1.0,1.0,1.0);
		glPointSize(3.0);
		glBegin(GL_LINE_LOOP);
			for (float angulo = 0.0; angulo<6.0; angulo+=0.0001){
				glVertex2f(radio*cos(angulo) + px,radio*sin(angulo) + py);
			}
		glEnd();
	glPopMatrix();

}

void dibujarMarcoVerde() {
	
	glPushMatrix();
	
		glLineWidth(2.0);
		glColor3f(0.0,1.0,0.0);
		
		glBegin(GL_LINE_LOOP);
			glVertex2f(-9.0,9.5);
			glVertex2f(9.0,9.5);
			glVertex2f(9.0,9.0);
			glVertex2f(-9.0,9.0);
		glEnd();
		
		glBegin(GL_LINE_LOOP);
			glVertex2f(9.0,9.5);
			glVertex2f(9.5,9.5);
			glVertex2f(9.5,-9.0);
			glVertex2f(9.0,-9.0);
		glEnd();
		
		glBegin(GL_LINE_LOOP);
			glVertex2f(-9.0,9.5);
			glVertex2f(-9.5,9.5);
			glVertex2f(-9.5,-9.0);
			glVertex2f(-9.0,-9.0);
		glEnd();

	glPopMatrix();

}

void dibujarBloques() {

	glPushMatrix();
	
		glLineWidth(2.0);
		glColor3f(1.0,0.0,0.0);

		float cx = -8.2;
		float cy = 6.0;

		for (int i = 0; i < 5; i++){
			for (int j = 0;j < 7;j++){
				glBegin(GL_LINE_LOOP);
					glVertex2f(cx,cy);
					glVertex2f(cx+1.5,cy);
					glVertex2f(cx+1.5,cy-0.5);
					glVertex2f(cx,cy-0.5);
				glEnd();
				cx = cx+ 2.5;
			}
			cx = -8.2;
			cy = cy -1.25;
		}

	glPopMatrix();

}

void changeViewport(int w, int h) {
  float aspectradio;
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  aspectradio = (float) w / (float) h;
  if (w <= h)
    glOrtho(-10,10,-10/aspectradio,10/aspectradio,1.0,-1.0);
  else
    glOrtho(-10*aspectradio,10*aspectradio,-10,10,1.0,-1.0);
}

void keyboard(int key, int x, int y)
{
    switch (key){
	/*case GLUT_KEY_LEFT:
		
	case GLUT_KEY_RIGHT:
		
		
		break;
	default:
		break;*/
	}
	
	glutPostRedisplay();

}

void render(){
  float aspectradio;
  glClearColor(0.0f, 0.0f, 0.0f ,1.0f); 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  GLfloat zExtent, xExtent, xLocal, zLocal;
    int loopX, loopZ;

  /* Render Grid 
  glPushMatrix();
    glColor3f( 0.0f, 0.7f, 0.7f );
    glBegin( GL_LINES );
    zExtent = DEF_floorGridScale * DEF_floorGridZSteps;
    for(loopX = -DEF_floorGridXSteps; loopX <= DEF_floorGridXSteps; loopX++ )
  {
  xLocal = DEF_floorGridScale * loopX;
  glVertex3f( xLocal, -zExtent, 0.0f );
  glVertex3f( xLocal, zExtent,  0.0f );
  }
    xExtent = DEF_floorGridScale * DEF_floorGridXSteps;
    for(loopZ = -DEF_floorGridZSteps; loopZ <= DEF_floorGridZSteps; loopZ++ )
  {
  zLocal = DEF_floorGridScale * loopZ;
  glVertex3f( -xExtent, zLocal, 0.0f );
  glVertex3f(  xExtent, zLocal, 0.0f );
  }
    glEnd();
    glPopMatrix();*/


//------------- Dibujamos PLATAFORMA -------------          
	dibujarPlataforma();

//------------- Dibujamos PELOTA -------------
	dibujarPelota();

//------------- Dibujamos MARCO -------------
	dibujarMarcoVerde();

//------------- Dibujamos BLOQUES -------------
	if (bloqueEspecial == true) {
		golpesEliminar = 2;
	}else 
		golpesEliminar = 1;

	glPushMatrix();
		
		dibujarBloques();

	glPopMatrix();


	
 

  glFlush();
  glutSwapBuffers();
}

int main (int argc, char** argv) {

  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

  glutInitWindowSize(800,600);

  glutCreateWindow("Opengl");

  glutReshapeFunc(changeViewport);
  glutDisplayFunc(render);
  
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    fprintf(stderr, "GLEW error");
    return 1;
  }
  
  glutSpecialFunc(keyboard);
  glutMainLoop();
  return 0;

}
