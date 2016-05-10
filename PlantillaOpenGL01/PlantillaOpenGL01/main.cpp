#include <iostream>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <math.h>
#include <stdlib.h>
#include <ctime>

//#include <GL/freeglut.h>
//#include <GL/gl.h>

bool inicial = true, //true para inicializar los bonus y espaciales una sola vez
     isLeftKeyPressed = false, isRightKeyPressed = false, isUpKeyPressed = false,
     velocidad = false, //bonus de velocidad activado o desactivado
     baseLarga = true;  //bonus de base activado o desactivado

float plataforma = 0.0,      //posicion de la plataforma
      pelota[2] = {0.0,0.0}, //posicion de la pelota
      bloques[5][7] = {{0,0,1,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,1,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}},//matriz para los bloques
      posInicial[2][7] = {{-0.1,0.0,0.35,-0.13,-0.1,-0.04,0.0}, {-0.2,0.0,0.2,0.1,0.1,-0.17,-0.1}}; //posicion inical con lo que explotan los peazos

int especiales[5] = {}, //arreglo para los bloques especiales 
    bonus[6] = {},      //arreglo para los bloques bonus 
    tipo[6] = {};       //arreglo para el tipo de bonus que tendrá cada bloque, 
                        // 0 tamaño de la plataforma, 1 velocidad de la pelota

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

/********************** ESPECIALES Y BONUS **********************/
// Busca si un bloque es especial o es un bonus 
bool buscar(int x, int array[], int n){
  bool esta = false;
  int i = 0;

  while (i < n){
    if (x == array[i]) {
      esta = true;
      break;
    }
    i++;
  }
  return esta;
}

void generarEspeciales(){
      
    int r;
    for (int i = 0; i < 5; i++){
        r = rand()%35;
        while ( buscar(r,especiales,5)) r = rand()%35;
        especiales[i] = r;

    //printf("Estos son los bloques especiales %d \n", r);
    }
}

void generarBonus(){
      
    int r,a;
    for (int i = 0; i < 6; i++){
        r = rand()%35;
        while ( buscar(r,bonus,6)) r = rand()%35;
        bonus[i] = r; 
        tipo[i] = rand()%2;
    }   
}

/*********************** FUNCIONES PARA DIBUJAR ***********************/
// -------- DIBUJOS PARA LA FORMA DE LOS BONUS  --------

void dibujarBonusVelocidad(float cxb, float cyb, float radio){ //
  glColor3f(1.0,0.5,0.0);
  /*glPointSize(3.0);
    glBegin(GL_LINE_LOOP);
    for (float angulo = 0.0; angulo<6.0; angulo+=0.0001){
      glVertex2f(radio1*cos(angulo) + cxb,radio*sin(angulo) + cyb);
        }
    glEnd();*/
  glBegin(GL_LINE_LOOP);
    glVertex2f(0.0,0.0);
    glVertex2f(0.7,0.0);
    glVertex2f(0.6,-0.5);
    glVertex2f(0.8,-0.5);
    glVertex2f(0.5,-1.0);
    glVertex2f(0.6,-0.6);
    glVertex2f(0.3,-0.6);
    glVertex2f(0.5,-0.2);
  glEnd();
}

void dibujarBonusTamBase(float cxb, float cyb, float radio){ // largo 0.8 en X, alto 0.2 en Y
  
  glColor3f(1.0,0.0,0.5);
  /*glPointSize(3.0);
    glBegin(GL_LINE_LOOP);
    for (float angulo = 0.0; angulo<6.0; angulo+=0.0001){
      glVertex2f(radio2*cos(angulo) + cxb,radio*sin(angulo) + cyb);
        }
    glEnd();*/
  glBegin(GL_LINE_LOOP);
    glVertex2f(0.0,-0.2);
    glVertex2f(0.0,-0.4);
    glVertex2f(0.8,-0.4);
    glVertex2f(0.8,-0.2);
  glEnd();
}

// -------------------- DIBUJOS BASE  ---------------------- 
void dibujarPlataforma() {
    float tam = 2.0;

    glPushMatrix();
        glTranslatef(0.0,-8.5,0.0); 
        glColor3f(0.0,0.0,1.0);

        if(baseLarga){
          glBegin(GL_LINES);
              glVertex2f(-tam+plataforma, 0.0);
              glVertex2f(-tam+plataforma, -0.5);
              glVertex2f(tam+plataforma, 0.0);
              glVertex2f(tam+plataforma, -0.5);
          glEnd();
          tam += 0.3;
        } // base = 4 -> 15% = 0.6 -> base = 4.6

        glBegin(GL_LINE_LOOP);
            glVertex2f(-tam+plataforma,0.0);
            glVertex2f(tam+plataforma,0.0);
            glVertex2f(tam+plataforma,-0.5);
            glVertex2f(-tam+plataforma,-0.5);
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

// --------DIBUJO CUANDO EL BLOQUE SE ROMPE--------
void dibujarCirculo(float px, float py) {
    float x,y,radio = 0.14;
    glPointSize(2.0);
    glBegin(GL_POINTS);
        for(double i=0.0; i<10; i+=0.001){
            x=radio*cos(i)+px;
            y=radio*sin(i)+py;
            glVertex2f(x,y);
        }
    glEnd();
}

void dibujarExplosion(float cx, float cy,float posiciones[2][7]){

    glPushMatrix();
        glTranslatef(cx+0.75,cy-0.25,0.0); 

        for (int i = 0; i < 7; i++){
            dibujarCirculo(posiciones[0][i], posiciones[1][i]);
            // se puede jugar con los valores para que el movimiento sea distinto
            if (i == 3) posiciones[0][i] = -1;
            else if (posiciones[0][i] >= 0) posiciones[0][i] += rand()%9 * 0.01;
            else posiciones[0][i] -= rand()%3 * 0.1;
            if (i == 6) posiciones[1][i] = 1.5;
            else if (i == 0) posiciones[1][i] = rand()%2 * 0.01;
            else if (posiciones[1][i] >= 0) posiciones[1][i] += rand()%9 * 0.01;
            else posiciones[1][i] -= rand()%3 * 0.1;
        }

        //glutTimerFunc(30,dibujarExplosion(cx, cy, posiciones, n++),-1);
        for (int i = 0; i < 10; i++){
          glutPostRedisplay();
        }

    glPopMatrix();
}
// --------FIN DIBUJO CUANDO EL BLOQUE SE ROMPE--------

void dibujarBloque(float cx, float cy, float color){

    glColor3f(1.0,color,0.0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cx,cy);
        glVertex2f(cx+1.5,cy);
        glVertex2f(cx+1.5,cy-0.5);
        glVertex2f(cx,cy-0.5);
    glEnd();

}

void dibujarBloqueRoto(float cx, float cy){ //hay que revisarlo

  glColor3f(0.8,1.0,0.8);
  glBegin(GL_LINE_LOOP);
    glVertex2f(cx,cy);
    glVertex2f(cx+1.5/2,cy);
    glVertex2f(cx+1.5/2-0.25,cy-0.15);
    glVertex2f(cx+1.5/2+0.15,cy-0.4);
    glVertex2f(cx+1.5/2,cy-0.5);
    glVertex2f(cx,cy-0.5);
  glEnd();
  glBegin(GL_LINE_LOOP);
    glVertex2f(cx+1.5/2+0.1,cy);
    glVertex2f(cx+1.5,cy);
    glVertex2f(cx+1.5,cy-0.5);
    glVertex2f(cx+1.5/2+0.15,cy-0.5);
    glVertex2f(cx+1.5/2+0.25,cy-0.4);
    glVertex2f(cx+1.5/2-0.1,cy-0.15);
  glEnd();

}

void dibujarBloques() {

    glPushMatrix();
    
        glLineWidth(2.0);

        float cx = -8.2;
        float cy = 6.0;

        for (int i = 0; i < 5; i++){
            for (int j = 0;j < 7;j++){
                if (bloques[i][j] == 0 && buscar((i*7+j),especiales,5)) //dibuja especiales 
                    dibujarBloque(cx, cy, 1);
                else if (bloques[i][j] == 1 && buscar(i*7+j,especiales,5)) //dibuja especiales golpeados una vez
                    dibujarBloqueRoto(cx, cy);
                else if(bloques[i][j] == 0) //dibuja el resto de los bloques que no han sido golpeados
                    dibujarBloque(cx, cy, 0);
                cx = cx+ 2.5;
            }
            cx = -8.2;
            cy = cy -1.25;
        }

    glPopMatrix();

}

//------- Sólo falta dibujo de impacto a bloque roto, para todos los demás ya está hecha la base.

/***************** FIN FUNCIONES PARA DIBUJAR *****************/

/************************* MOVIMIENTO *************************/

void moverPelota(int h){
  if (h > 0){
    glPushMatrix();

      int anguloRotacion = 45;
      // Rotación respecto al centro de la pelota
      float x = pelota[0]-5,y = pelota[1]-2;
      while (x < 20){
        glTranslatef(x,y,0);
        glRotatef(anguloRotacion,0.0,0.0,1);
        glTranslatef(-pelota[0],-pelota[1],0);
        dibujarPelota(0.3);
        x += 1;
        y += 1;
      }
      //anguloRotacion += 5;
      

    glPopMatrix();

  }
  /* //--- Completar cuando funcione bien el lado derecho!
  else if (h < 0){
    glPushMatrix();

      int anguloRotacion = -45;
      glTranslatef(pelota[0],pelota[1]-2,0);
      glRotatef(anguloRotacion,0.0,0.0,1);
      glTranslatef(-pelota[0],-pelota[1],0);
      dibujarPelota(0.3);
      //anguloRotacion -= 5;
    glPopMatrix();
  }
  */
}

void handleSpecialKeypress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            isLeftKeyPressed = true;
            if (!isRightKeyPressed) {
                if (plataforma > -6.6 && !baseLarga) plataforma -= 0.2;
                else if (plataforma > -6.8 && !baseLarga) plataforma -= 0.1;
                else if (plataforma > -6.4 && baseLarga) plataforma -= 0.2;
            }
        break;
        case GLUT_KEY_RIGHT:
            isRightKeyPressed = true;
            if (!isLeftKeyPressed) {
                if (plataforma < 6.6 && !baseLarga) plataforma += 0.2;
                else if (plataforma < 6.8 && !baseLarga) plataforma += 0.1;
                else if (plataforma < 6.4 && baseLarga) plataforma += 0.2;
            }
        break;
    
    }
  glutPostRedisplay(); // Esto debe ir, si no, no funciona 
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
  // glutPostRedisplay(); Creo que en este caso no es necesario
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

  if(inicial) {
    srand(time(NULL)); //genera semilla basada en el reloj del sistema
    generarEspeciales();
    generarBonus();
    inicial = false;
  }

//------------- Dibujamos PLATAFORMA -------------          
    dibujarPlataforma();

//------------- Dibujamos PELOTA -------------
    dibujarPelota(0.3);
    dibujarExplosion(2, 2,posInicial);
    /*
  if (!isLeftKeyPressed && !isRightKeyPressed) 

  glPushMatrix();
    glColor3f(1.0,0.0,1.0);
    glTranslatef(0.0,-8.1,0.0); 
    glPointSize(3.0);
    glBegin(GL_POINTS);
      glVertex2f(0.0+0.3,0.0);
      glVertex2f(0.0-0.3,0.0);
      glVertex2f(0.0,0.0+0.3);
      glVertex2f(0.0,0.0-0.3);
    glEnd();
  glPopMatrix();
*/  

//------------- Dibujamos MARCO -------------
    dibujarMarcoVerde();

//------------- Dibujamos BLOQUES -------------
    
    dibujarBloques();

//------------- Dibujo Bonus para probar -------------

  //dibujarBonusTamBase(0.4,-0.3,0.5);
  //dibujarBonusVelocidad(0.4,-0.3,0.5);
  
  /*
  if (!isLeftKeyPressed && isRightKeyPressed) {
    int w = 1;
    moverPelota(w);
  }

  if (isLeftKeyPressed && !isRightKeyPressed) {
    int s = -1;
    moverPelota(s);
  }
  */
    
 

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
    f//printf(stderr, "GLEW error");
    return 1;
    }
    */


    glutSpecialFunc(handleSpecialKeypress);
    glutSpecialUpFunc(handleSpecialKeyReleased);


    glutMainLoop();
    return 0;

}