#include <iostream>
//#include <GL\glew.h>
//#include <GL\freeglut.h>
#include <math.h>
#include <stdlib.h>

#include <GL/freeglut.h>
#include <GL/gl.h>

float radio = 0.3, px = 0.0, py = -8.0; // variables de ayuda para dibujar la pelota.

bool bloqueEspecial = true; // variable booleana para determinar si un bloque es especial o no
                            // true si el bloque es especial, false si no.

int golpesEliminar = 0; // variable para saber con cuantos golpes se elimina la pelota.

bool bloqueBonus = false; // variable booleana para determinar si un bloque tiene bonus o no
                          // true si el bloque tiene bonus, false si no.

bool inicial = true, isLeftKeyPressed = false, isRightKeyPressed = false;

float plataforma = 0.0 ; // posicion de la plataforma
float pelota[2] = {0.0,0.0}; //posicion de la pelota

float bloques[5][7] = {{0,1,0,0,0,0,0},{0,0,1,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,1,0,0,0,1,0}}; //matriz para los bloques (creo que podria ser de int)
int especiales[5] = {1,11,34,2,9}; //arreglo para los bloques especiales (creo que podria ser de int)
float bonus[6] = {};      //arreglo para los bloques bonus (creo que podria ser de int)


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

bool buscarEspecial(int x){
    bool esta = false;
    int i = 0;

    while (i < 5){
        if (especiales[i] == x) {
            esta = true;
            break;
        }
        i++;
    }

    return esta;
}

void generarEspeciales(){
    srand(time(0)); //genera semilla basada en el reloj del sistema
    int r;
    for (int i = 0; i < 5; i++){
        r = rand()%35;
        while( buscarEspecial(r) ) r = rand()%35;
        especiales[i] == r;
    }

    inicial = false;
}

void dibujarPlataforma() {

    glPushMatrix();
        glTranslatef(0.0,-8.5,0.0); 
        glColor3f(0.0,0.0,1.0);
        glBegin(GL_LINE_LOOP);
            glVertex2f(-2.0+plataforma,0.0);
            glVertex2f(2.0+plataforma,0.0);
            glVertex2f(2.0+plataforma,-0.5);
            glVertex2f(-2.0+plataforma,-0.5);
        glEnd();
    glPopMatrix();

}

void dibujarPelota(float radio) {

    glPushMatrix();
        glTranslatef(0.0,-8.1,0.0); 
        glColor3f(1.0,1.0,1.0);
        glPointSize(3.0);
        glBegin(GL_LINE_LOOP);
            for (float angulo = 0.0; angulo<6.0; angulo+=0.0001){
                glVertex2f(radio*cos(angulo) + pelota[0],radio*sin(angulo) + pelota[1]);
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

void dibujarBloque(float cx, float cy, float color){

    glColor3f(1.0,color,0.0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cx,cy);
        glVertex2f(cx+1.5,cy);
        glVertex2f(cx+1.5,cy-0.5);
        glVertex2f(cx,cy-0.5);
    glEnd();

}

void dibujarBloqueRoto(float cx, float cy){ //hay que dibujarlo distinto

    glColor3f(0.0,1.0,0.0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cx,cy);
        glVertex2f(cx+1.5,cy);
        glVertex2f(cx+1.5,cy-0.5);
        glVertex2f(cx,cy-0.5);
    glEnd();

}

void dibujarBloques() {

    glPushMatrix();
    
        glLineWidth(2.0);

        float cx = -8.2;
        float cy = 6.0;

        for (int i = 0; i < 5; i++){
            for (int j = 0;j < 7;j++){
                if ( bloques[i][j] == 0 && buscarEspecial(i*7+j) )
                    dibujarBloque(cx, cy, 1);
                else if ( bloques[i][j] == 1 && buscarEspecial(i*7+j) )
                    dibujarBloqueRoto(cx, cy);
                else if(bloques[i][j] == 0) 
                    dibujarBloque(cx, cy, 0);
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

void handleSpecialKeypress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            isLeftKeyPressed = true;
            if (!isRightKeyPressed) {
                if (plataforma > -6.2)
                    plataforma -= 1;
            }
        break;
        case GLUT_KEY_RIGHT:
            isRightKeyPressed = true;
            if (!isLeftKeyPressed) {
                if (plataforma < 6.2)
                    plataforma += 1;
            }
        break;
    }
}

void handleSpecialKeyReleased(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            isLeftKeyPressed = false;
        break;
        case GLUT_KEY_RIGHT:
            isRightKeyPressed = false;
        break;
    }
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

    if(inicial) generarEspeciales();

//------------- Dibujamos PLATAFORMA -------------          
    dibujarPlataforma();

//------------- Dibujamos PELOTA -------------
    dibujarPelota(0.3);

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

    /*
    GLenum err = glewInit();
    if (GLEW_OK != err) {
    fprintf(stderr, "GLEW error");
    return 1;
    }
    */


    glutSpecialFunc(handleSpecialKeypress);
    glutSpecialUpFunc(handleSpecialKeyReleased);


    glutMainLoop();
    return 0;

}
