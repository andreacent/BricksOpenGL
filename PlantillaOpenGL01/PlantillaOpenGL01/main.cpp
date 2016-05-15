#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <ctime>
#include <stdio.h>
//#include <GL\glew.h>
//#include <GL\freeglut.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glut.h>

using namespace std;

#define DEF_floorGridScale  1.0f
#define DEF_floorGridXSteps 10.0f
#define DEF_floorGridZSteps 10.0f

#define PI 3.14159265358979f
#define lb 1.9    // largo del bloque
#define ab 0.5    // altura del bloque
#define vp 0.2    // velocidad con la que se mueve la plataforma
#define vb 0.014  // velocidad con la que caen los bonos
#define ve 0.01   // velocidad de la explosion
#define cbe 5     // cantidad de bloques especiales
#define cbb 6     // cantidad de bloques con bonus

bool isLeftKeyPressed = false, isRightKeyPressed = false,
     inicial = true,        //true para inicializar los bonus y especiales una sola vez
     velocidad = false,     //bonus de velocidad activado o desactivado
     baseLarga = false,     //bonus de base activado o desactivado
     moviendose = false,    //true si la pelota comienza a moverse
     gameOver = false;      

float // variables para cuando el bloque se rompe
      posInicial[8][2] = {{1,0},{0,0},{0,1},{0,0},{1,0},{1,0},{1,0},{0,0}}, 
      anguloPedazos[8] = {-330,-280,-90,-180,220,-60,320,-100},
      posBonus[6][2],       //posicion de los bonos
      posEspeciales[5][3],  //posicion de los bonos y suma
      // PLATAFORMA
      plataforma = 0.0, //posicion
      tam = 2.0;             

int bloques[5][7] = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},
                       {0,0,0,0,0,0,0},{0,0,0,0,0,0,0}}, //matriz de bloques
    destruidos=0,    //cantidad de bloques destruidos
    especiales[5],   //arreglo para los bloques especiales 
    bonus[6][2];     //bloque y tipo de bonus: 0 velocidad pelota, 1 tam plataforma 

// PELOTA
GLfloat radioP = 0.3f;            // Radio de la pelota.
GLfloat xSpeed = 0.08f;           // Velocidad en X y Direccion en Y.
GLfloat ySpeed = 0.05f;
GLfloat anguloP = 40.0f;          // Angulo con el que se mueve la pelota;
GLfloat velocidadP = 0.00001;     // Velocidad de la pelota
GLfloat pelota[2] = {0.0f,0.0f};  // Centro de la pelota.

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
int buscarEspeciales(int x){
  int esta = -1;

  for (int i = 0; i < cbe; i++){
    if (x == especiales[i]) {
      esta = i;
      break;
    }
  }
  return esta;
}

void generarEspeciales(){
  int i,j,r;
  for (int k = 0; k < cbe; k++){
    do{
        i = rand()%5;
        j = rand()%7;
        r = i*7+j;
    }while ( buscarEspeciales(r) > -1 );
    especiales[k] = r;
    posEspeciales[k][0] = j*2.5 - 8.4 + 1; // calcula posicion en x
    posEspeciales[k][1] = 15 - i*1.25 - 0.25; // calcula posicion en y
  }
}

int buscarBonus(int x){
  int esta = -1;

  for (int i = 0; i < cbb; i++){
    if (x == bonus[i][0]) {
      esta = i;
      break;
    }
  }
  return esta;
}

void generarBonus(){
  int i,j,r;
  for (int k = 0; k < cbb; k++){
    do{
        i = rand()%5;
        j = rand()%7;
        r = i*7+j;
    }while ( buscarBonus(r) >= 0 );
    bonus[k][0] = r;
    bonus[k][1] = rand()%2;
    posBonus[k][0] = j*2.5 - 8.4; // calcula posicion en x
    posBonus[k][1] = 15 - i*1.25; // calcula posicion en y
  }   
}

/*********************** FUNCIONES PARA DIBUJAR ***********************/
// --------DIBUJO CUANDO EL BLOQUE ESPECIAL SE ROMPE--------
void dibujarCirculo(float px, float py, float radio = 0.14, float col = 1) {
    float x,y;
    glPointSize(2.0);
    glColor3f(1.0,1.0,col);
    glBegin(GL_POINTS);
        for(double i=0.0; i<10; i+=0.001){
            x=radio*cos(i)+px;
            y=radio*sin(i)+py;
            glVertex2f(x,y);
        }
    glEnd();
}

void dibujarExplosion(int b){
  float x, y;
  glPushMatrix();
    glTranslatef(posEspeciales[b][0],posEspeciales[b][1],0.0); 
    for (int i = 0; i < 8; i++){    
      if(posInicial[i][0] > 0)
        x = 0.01*cos(anguloPedazos[i])+ (posInicial[i][0]+posEspeciales[b][2]); 
      else 
        x = 0.01*cos(anguloPedazos[i])+ (posInicial[i][0]-posEspeciales[b][2]);
      if(posInicial[i][1] > 0)
        y = 0.01*sin(anguloPedazos[i])+ (posInicial[i][1]+posEspeciales[b][2]);
      else 
        y = 0.01*sin(anguloPedazos[i])+ (posInicial[i][1]-posEspeciales[b][2]);
      dibujarCirculo(x, y);
      posEspeciales[b][2] += ve;
    }
  glPopMatrix();  
}

// -------- DIBUJOS PARA LA FORMA DE LOS BONUS  --------
void dibujarBonusVelocidad(float x, float y, int bono){ 
  glPushMatrix();
    glTranslatef(x,y,0.0);
    glColor3f(1.0,0.5,0.0);
    glBegin(GL_LINE_LOOP);
      glVertex2f(0,0);
      glVertex2f(0.7,0);
      glVertex2f(0.6,-0.5);
      glVertex2f(0.8,-0.5);
      glVertex2f(0.5,-1.0);
      glVertex2f(0.6,-0.6);
      glVertex2f(0.3,-0.6);
      glVertex2f(0.5,-0.2);
    glEnd();
  glPopMatrix();

  // Colision con la plataforma
  if(  (y-1 <= -0.3 && tam+plataforma >= x+0.5 && -tam+plataforma <= x+0.5)   //(0.5,-1) dentro de la plataforma
    || (y-0.5 <= -0.3 && tam+plataforma >= x+0.8 && -tam+plataforma <= x+0.8) //(0.8,-0.5) dentro de la plataforma
    || (y <= -0.3 && tam+plataforma >= x+0.7 && -tam+plataforma <= x+0.7)     //(0.7,0) dentro de la plataforma
    || (y <= -0.3 && tam+plataforma >= x && -tam+plataforma <= x)             //(0,0) dentro de la plataforma
    || (y-0.6 <= -0.3 && tam+plataforma >= x+0.3 && -tam+plataforma <= x+0.3) //(0.3,-0.6) dentro de la plataforma
    || (y-0.6 <= -0.3 && tam+plataforma <= x+0.6 && tam+plataforma >= x+0.5))   
  {
    printf("bonus Velocidad\n");
    if(!velocidad){
      velocidad = true;
      velocidadP += velocidadP*0.4; // aumenta velocidad 40%
      xSpeed += xSpeed*0.4;
      ySpeed += ySpeed*0.4;
    }
    posBonus[bono][1] = -10;
  }
}

void dibujarBonusTamBase(float x, float y, int bono){ 

    glPushMatrix();
      glTranslatef(x,y,0.0);
      glColor3f(1.0,0.0,0.5);
      glBegin(GL_LINE_LOOP);
        glVertex2f(2,-0.2);
        glVertex2f(2,-0.4);
        glVertex2f(1,-0.4);
        glVertex2f(1,-0.2);
      glEnd();
    glPopMatrix();

    // Colision con la plataforma
    if(-tam+plataforma <= x+1.0 && tam+plataforma >= x+0.2 && y-0.4 <= -0.3){
      printf("bonus TamBase\n");
      if(!baseLarga){
        tam -= tam*0.15;   // disminuyes tam 15%
        baseLarga = true;
      }
      posBonus[bono][1] = -10;
    }
}

// -------------------- DIBUJOS BASE  ---------------------- 
void dibujarPlataforma(){

    if( -tam+plataforma < -8.9) plataforma -= -tam+plataforma + 8.9;
    else if(tam+plataforma > 8.9) plataforma -= tam+plataforma - 8.9;

    glPushMatrix();
        glTranslatef(0.0,-8.2,0.0); 
        glColor3f(0.0,0.0,1.0);

        glBegin(GL_LINE_LOOP);
            glVertex2f(-tam+plataforma,-0.3);
            glVertex2f(tam+plataforma,-0.3);
            glVertex2f(tam+plataforma,-0.8);
            glVertex2f(-tam+plataforma,-0.8);
        glEnd();
    glPopMatrix();
}

void dibujarPelota(float r) {
    glLineWidth(1.0);
    glPushMatrix();
    if (velocidadP > 0.0){
      pelota[0] = pelota[0] + xSpeed*velocidadP*cos(anguloP);
      pelota[1] = pelota[1] + ySpeed*velocidadP*sin(anguloP);
    }
    glTranslatef(0.0,-8.2,0.0); 
    glColor3f(1.0,1.0,1.0);
    glPointSize(3.0);
    glBegin(GL_LINE_LOOP);
        for (float angulo = 0.0; angulo<6.0; angulo+=0.0001){
            glVertex2f(r*cos(angulo) + pelota[0],r*sin(angulo) + pelota[1]);
        }
    glEnd();

    pelota[0] += xSpeed;
    pelota[1] += ySpeed;

    // Colision con paredes
    if (pelota[0] + radioP >= 8.9){    
      pelota[0] = 8.9 - radioP;
      xSpeed = -xSpeed;
    }
    else if(pelota[0] - radioP <= -8.9){
      pelota[0] = -8.9 + radioP;
      xSpeed = -xSpeed;
    }

    else if (pelota[1] + radioP >= 17.1){
      pelota[1] = 17.1 - radioP;
      ySpeed = -ySpeed;
    }
    pelota[0] = pelota[0] + xSpeed*velocidadP*cos(anguloP);
    pelota[1] = pelota[1] + ySpeed*velocidadP*sin(anguloP);

    // Colision con la plataforma
    
    if ((-tam+plataforma <= pelota[0]) && (pelota[0] <= tam+plataforma)
        && (pelota[1] - radioP <= -0.3)){
        ySpeed = -ySpeed;
        pelota[1] += radioP;
    }

    else if ((pelota[0] + radioP >= -tam+plataforma) && (pelota[0] + radioP < tam+plataforma)
        && (pelota[1] <= -0.3) && (pelota[1] >= -0.8)){
        xSpeed = -xSpeed;
        pelota[0] = -tam+plataforma -radioP;
    }

    else if ((pelota[0] - radioP <= tam+plataforma) && (pelota[0] - radioP > -tam+plataforma)
        && (pelota[1] <= -0.3) && (pelota[1] >= -0.8)){
        xSpeed = -xSpeed;
        pelota[0] = tam+plataforma + radioP;
    }
    else if (pow((-tam+plataforma-pelota[0]),2) + pow(-0.3-pelota[1],2) <= pow(radioP,2)){
        ySpeed = -ySpeed;
        pelota[1] += radioP;
    }
    else if (pow((tam+plataforma-pelota[0]),2) + pow(-0.3-pelota[1],2) <= pow(radioP,2)){
        ySpeed = -ySpeed;
        pelota[1] += radioP;
    }
    else if (pow((tam+plataforma-pelota[0]),2) + pow(-0.8-pelota[1],2) <= pow(radioP,2)){
        xSpeed = -xSpeed;
        pelota[0] += radioP;
    }
    else if (pow((-tam+plataforma-pelota[0]),2) + pow(-0.8-pelota[1],2) <= pow(radioP,2)){
        xSpeed = -xSpeed;
        pelota[0] += radioP;
    }
    // El jugador pierde
    else if(pelota[1] < -0.6) gameOver = true;        

    pelota[0] = pelota[0] + xSpeed*velocidadP*cos(anguloP);
    pelota[1] = pelota[1] + ySpeed*velocidadP*sin(anguloP);

    glPopMatrix();

    glLineWidth(2.0);
}

void dibujarMarcoVerde() { 
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
}

// -----------------------BLOQUES-----------------------
void dibujarBloque(float cx, float cy, float color){

  glColor3f(1.0,color,0.0);
  glPushMatrix();
    glTranslatef(cx,cy,0.0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(0,0);
        glVertex2f(lb,0);
        glVertex2f(lb,-ab);
        glVertex2f(0,-ab);
    glEnd();
  glPopMatrix();

}

void dibujarBloqueRoto(float cx, float cy){ 

  glColor3f(0.8,1.0,0.8);
  glLineWidth(1.5);
  glPushMatrix();
    glTranslatef(cx,cy,0.0);

    glBegin(GL_LINE_LOOP);
      glVertex2f(0,0);
      glVertex2f(lb/2,0);
      glVertex2f(lb/2-0.25,-0.15);
      glVertex2f(lb/2+0.15,-0.4);
      glVertex2f(lb/2,-ab);
      glVertex2f(0,-ab);
    glEnd();

    glBegin(GL_LINE_LOOP);
      glVertex2f(lb/2+0.1,0);
      glVertex2f(lb,0);
      glVertex2f(lb,-ab);
      glVertex2f(lb/2+0.15,-ab);
      glVertex2f(lb/2+0.25,-0.4);
      glVertex2f(lb/2-0.1,-0.15);
    glEnd();

    glBegin(GL_LINES);
      glVertex2f(lb/2-0.25,-0.15);
      glVertex2f(0,-0.15);
      glVertex2f(lb/2+0.15,-0.4);
      glVertex2f(0,-0.4);
      glVertex2f(lb/2+0.25,-0.4);
      glVertex2f(lb,-0.4);
      glVertex2f(lb/2-0.1,-0.15);
      glVertex2f(lb,-0.15);
    glEnd();

  glPopMatrix();
  glLineWidth(2.0);
}

bool hayChoque(float x, float y){
    bool choca = false;
    float px = pelota[0], py = pelota[1];

    if (px-radioP <= x+lb && px-radioP > x && py <= y && py >= y-ab) {// choca del lado der del bloque
      xSpeed = -xSpeed;
      px = x + lb + radioP;
      choca = true;
    }
    else if (px+radioP >= x && px+radioP < x +lb && py <= y && py >= y-ab) {// choca del lado izq del bloque
      xSpeed = -xSpeed;
      px = x - radioP;
      choca = true;
    }
    else if (px >= x && px < x+lb && y >= py-radioP && py-radioP >= y-ab) {//  choca de la parte de arriba del bloque
      ySpeed = -ySpeed;
      py = y + radioP;
      choca = true;
    } 
    else if (x <= px && px <= x+lb &&  py+radioP <= y && y-ab <= py+radioP) {// choca de la parte de abajo del bloque
      ySpeed = -ySpeed;
      py = y - ab - radioP;
      choca = true;
    }
    else if(( pow ((x-px),2) + pow(y-py,2) == pow (radioP,2))         //  choca con la esquina sup izq bloque
           || (pow((x+lb-px),2) + pow (y-py,2) == pow(radioP,2))){  //  choca con la esquina sup der bloque
      ySpeed = -ySpeed;
      xSpeed = -xSpeed;
      choca = true;
    }

    else if (( pow ((x-px),2) + pow(y-ab-py,2) == pow (radioP,2))          //  choca con la esquina inf izq bloque
            || (pow((x+lb-px),2) + pow (y-ab-py,2) == pow(radioP,2))) {//  choca con la esquina inf der bloque
      xSpeed = -xSpeed;
      ySpeed = -ySpeed;
      choca = true;
    }
    pelota[0] = pelota[0] + xSpeed*velocidadP*cos(anguloP);
    pelota[1] = pelota[1] + ySpeed*velocidadP*sin(anguloP);

    return choca;
}

void dibujarBloques() {
  int esBonus, esEspecial;
  float cx = -8.4, cy = 15;

  glPushMatrix();
    glTranslatef(0.0,-8.2,0.0); //comparte eje con la pelota para revisar las colisiones mas facil

    for (int i = 0; i < 5; i++){
      for (int j = 0;j < 7;j++){

        if (bloques[i][j] > -1){
          if(hayChoque(cx,cy)) bloques[i][j] +=1; //revisa si hay un choque
        }              

        esEspecial = buscarEspeciales(i*7+j);

        switch (bloques[i][j]) {
          case -1: //bloque eliminado 
            esBonus = buscarBonus(i*7+j);
            if (esBonus > -1 && posBonus[esBonus][1] > 0){
              switch (bonus[esBonus][1]) {
                case 0:
                  dibujarBonusVelocidad(posBonus[esBonus][0],posBonus[esBonus][1],esBonus);    
                break;
                case 1:
                  dibujarBonusTamBase(posBonus[esBonus][0],posBonus[esBonus][1],esBonus);
                break;
              }
              if(posBonus[esBonus][1] > -9) posBonus[esBonus][1] -= vb; 
            }
            if(esEspecial > -1 && posEspeciales[esEspecial][2] < 50){
              dibujarExplosion(esEspecial);
            }
          break;
          case 0: //el bloque no hay sido golpeado
            if(esEspecial > -1) dibujarBloque(cx, cy, 1);//dibuja especiales
            else dibujarBloque(cx, cy, 0);    
          break;
          case 1: // bloques golpeados una vez
            if(esEspecial > -1) dibujarBloqueRoto(cx, cy);
            else{
              bloques[i][j] = -1;
              destruidos +=1;
            }
          break;
          case 2: //bloques golpeados dos veces
            if(esEspecial > -1){              
              bloques[i][j] = -1;
              destruidos +=1; 
            }
          break;
        }
        cx += 2.5;
      }
      cx = -8.4;
      cy -= 1.25;
    }
  glPopMatrix();
}

/************DIBUJA UNA CARA SI PIERDE O GANA************/
void dibujarCara(){
    float x,y;
    glColor3f(1,1,0);
    dibujarCirculo(0.0, 0.0, 6,0); // radio mas grandee

    glPointSize(30.0);
    //ojos
    glBegin(GL_POINTS);
        glVertex2f(-2.3,2.3);
        glVertex2f(2.3,2.3);
    glEnd();

    glPointSize(2.0);
    glPushMatrix();
    glScalef(0.8,0.8,0.8);

    if(gameOver){
        glTranslatef(0,-3.6,0);
        glBegin(GL_POINTS);
            for(double i=0.0; i<3; i+=0.001){
                x=4*cos(i);
                y=4*sin(i);
                glVertex2f(x,y);
            }
        glEnd();
    }
    else{
        glTranslatef(0,-0.5,0);
        glRotatef(180,0,0,1);
        glBegin(GL_POINTS);
            for(double i=0.0; i<3; i+=0.001){
                x=4*cos(i);
                y=4*sin(i);
                glVertex2f(x,y);
            }
        glEnd();
    }
    glPopMatrix();
}

/***************** FIN FUNCIONES PARA DIBUJAR *****************/
/************************* MOVIMIENTO *************************/
void movimientoPelota(int h){
  if (h > 0){
    pelota[0] = velocidadP*cos(anguloP)+ pelota[0];
    pelota[1] = velocidadP*sin(anguloP)+ pelota[1];
    glutTimerFunc(10,movimientoPelota,1);
    glutPostRedisplay();
  }
}

void handleSpecialKeypress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            isLeftKeyPressed = true;
            if (!isRightKeyPressed) {
                plataforma -= vp;
                if (!moviendose){
                  anguloP = 95 + rand()% 25;
                  glutTimerFunc(10,movimientoPelota,1);
                  moviendose = true;
                } 
            }
        break;
        case GLUT_KEY_RIGHT:
            isRightKeyPressed = true;
            if (!isLeftKeyPressed) {
                plataforma += vp;
                if (!moviendose){
                  anguloP = 40 + rand()% 39;
                  glutTimerFunc(10,movimientoPelota,1);
                  moviendose = true;
                } 
            }
        break;
    }
    glutPostRedisplay();
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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH); 
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  glLineWidth(2.0);

  if(inicial) {
    srand(time(NULL)); //genera semilla basada en el reloj del sistema
    generarEspeciales();
    generarBonus();
    inicial = false;
  }

  if(destruidos < 35 && !gameOver){
    //------------- Dibujamos PLATAFORMA -------------          
        dibujarPlataforma();
    //------------- Dibujamos PELOTA -------------
        dibujarPelota(0.3);
    //------------- Dibujamos MARCO -------------
        dibujarMarcoVerde();
    //------------- Dibujamos BLOQUES -------------
        dibujarBloques();
  }else{
    dibujarCara();
    //printf("Puntos: %d\n", destruidos);
    //glutLeaveMainLoop();
  }

  glFlush();
  glutSwapBuffers();
}

int main (int argc, char** argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Bricks");
    glutReshapeFunc(changeViewport);
    glutDisplayFunc(render);

    //GLenum err = glewInit();
    //if (GLEW_OK != err) {
    //  printf(stderr, "GLEW error");
    //  return 1;
    //}

    glutSpecialFunc(handleSpecialKeypress);
    glutSpecialUpFunc(handleSpecialKeyReleased);
    
    glutMainLoop();

    return 0;
}