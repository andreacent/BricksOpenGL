#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <ctime>
#include <stdio.h>
#include <string.h>
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
#define vp 0.3    // velocidad con la que se mueve la plataforma
#define vb 0.014  // velocidad con la que caen los bonos
#define ve 0.008  // velocidad de la explosion
#define cbe 5     // cantidad de bloques especiales
#define cbb 6     // cantidad de bloques con bonus
#define cpb 8    // cantidad de pedazos cuando se rompe bloque

bool isLeftKeyPressed = false, isRightKeyPressed = false,
     inicial = true,        //true para inicializar los bonus y especiales una sola vez
     moviendose = false,    //true si la pelota comienza a moverse
     pausado = false,   
     gameOver = false;      

float // variables para cuando el bloque se rompe
      posInicial[cpb][2] = {{0.1,0.1},{0,0},{0,0.1},{0.1,0},
                            {0.1,-0.1},{0,0},{0,0.1},{0.1,0.1}}, 
      posBonus[cbb][2] = {},       //posicion de los bonos
      posEspeciales[cbe][3] = {},  //posicion de los bonos y suma
      // PLATAFORMA
      plataforma = 0.0, //posicion
      tam = 2.0;             

int bloques[5][7] = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},
                       {0,0,0,0,0,0,0},{0,0,0,0,0,0,0}}, //matriz de bloques
    destruidos=0,    //cantidad de bloques destruidos
    especiales[cbe],   //arreglo para los bloques especiales 
    bonus[cbb][2] ={};     //bloque y tipo de bonus: 0 velocidad pelota, 1 tam plataforma 

// PELOTA
GLfloat radioP = 0.3f;            // Radio de la pelota.
GLfloat xSpeed = 0.08f;           // Velocidad en X y Direccion en Y.
GLfloat ySpeed = 0.05f;
GLfloat velocidadP = 0.00001;     // Velocidad de la pelota
GLfloat pelota[2]  = {0.0f,0.0f}; // Centro de la pelota.


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

// -------------------------------TEXTO-------------------------------
typedef enum { MODE_BITMAP } mode_type;

static mode_type mode;
static int font_index;

void imprimir_bitmap_string(void* font, const char* s){
   if (s && strlen(s)) {
      while (*s) {
         glutBitmapCharacter(font, *s);
         s++;
      }
   }
}

void my_init(){
   mode = MODE_BITMAP;
   font_index = 0;
}

void dibujarTexto(int n) {
  void* bitmap_fonts[2] = {
    GLUT_BITMAP_9_BY_15,
    GLUT_BITMAP_HELVETICA_10,   
  };

  const char* bitmap_font_names[4] = {
    "¡FELICIDADES!",
    "PERDISTE",  
    "Usa las flechas <- y -> para comenzar",
    "Presiona Space para reanudar el juego",
  };

  glColor3f(0.0,0.0,1.0);

  switch (n) {
    case 0:
      glRasterPos2f(-2,-9.0);
      imprimir_bitmap_string(bitmap_fonts[0], bitmap_font_names[n]);   
    break;
    case 1:
      glRasterPos2f(-1.5,-9.0);
      imprimir_bitmap_string(bitmap_fonts[0], bitmap_font_names[n]);
    break;
    case 2:
      glRasterPos2f(-5.5,8);
      imprimir_bitmap_string(bitmap_fonts[0], bitmap_font_names[n]);
    break;
    case 3:
      glRasterPos2f(-5.5,8);
      imprimir_bitmap_string(bitmap_fonts[0], bitmap_font_names[n]);
    break;
    default: break;
  }

}
// ----------------------------FIN TEXTO----------------------------


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
void dibujarCirculo(float px, float py, float radio = 0.1, float col = 1) {
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
    for (int i = 0; i < (int)cpb/2; i++){    
      if(posInicial[i][0] > 0)
        x = posInicial[i][0]+posEspeciales[b][2]+1.0; 
      else 
        x = posInicial[i][0]-posEspeciales[b][2]-2.0;
      if(posInicial[i][1] > 0)
        y = posInicial[i][1]+posEspeciales[b][2];
      else 
        y = posInicial[i][1]-posEspeciales[b][2]-0.25;
      dibujarCirculo(x, y);
      posEspeciales[b][2] += ve;
    }
    for (int i = (int)cpb/2; i < cpb; i++){    
      if(posInicial[i][0] > 0)
        x = posInicial[i][0]+posEspeciales[b][2]+2.0;
      else 
        x = posInicial[i][0]-posEspeciales[b][2]-0.5;
      if(posInicial[i][1] > 0)
        y = posInicial[i][1]+posEspeciales[b][2]+0.25;
      else 
        y = posInicial[i][1]-posEspeciales[b][2]-1.5;
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
    velocidadP += velocidadP*0.4; // aumenta velocidad 40%
    xSpeed += xSpeed*0.4;
    ySpeed += ySpeed*0.4;
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
    if((-tam+plataforma <= x+1.0 && tam+plataforma >= x+0.2 && y-0.4 <= -0.3)
      || (-tam+plataforma <= x+0.2 && y-0.4 <= -0.3 && tam+plataforma >= x+1.0))
    {
      printf("bonus TamBase\n");
      tam -= tam*0.15;   // disminuyes tam 15%
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
    glTranslatef(0.0,-8.2,0.0); 
    glColor3f(1.0,1.0,1.0);
    glPointSize(3.0);
    glBegin(GL_LINE_LOOP);
        for (float angulo = 0.0; angulo<6.0; angulo+=0.0001){
            glVertex2f(r*cos(angulo) + pelota[0],r*sin(angulo) + pelota[1]);
        }
    glEnd();
  glPopMatrix();
  glLineWidth(2.0);

  //revisamos colisiones
  if(!pausado && moviendose){
      pelota[0] += xSpeed;
      pelota[1] += ySpeed;
        
      // El jugador pierde
      if(pelota[1] < -0.8) gameOver = true; 
      // Colision con paredes
      else if (pelota[0] + radioP >= 8.9){    
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
      // Colision con la plataforma      
      else if ((pow((plataforma-tam-pelota[0]),2) + pow(-0.3-pelota[1],2) <= pow(radioP,2))
              || (pow((tam+plataforma-pelota[0]),2) + pow(-0.3-pelota[1],2) <= pow(radioP,2)))
      {//esquinas
          xSpeed = -xSpeed;
          ySpeed = -ySpeed;
          pelota[1] += radioP;
      }  
      else if(pelota[0]+radioP >= plataforma-tam 
              && pelota[0]+radioP < plataforma
              && pelota[1] <= -0.3){ //la pelota choca con lado izq
          xSpeed = -xSpeed;
          pelota[0] = -tam+plataforma-radioP;
      }
      else if(pelota[0]-radioP <= tam+plataforma 
              && pelota[0]-radioP > plataforma 
              && pelota[1] <= -0.3){//la pelota choca con lado der
          xSpeed = -xSpeed;
          pelota[0] = tam+plataforma+radioP;       
      }
      else if (plataforma-tam <= pelota[0] && pelota[0] <= tam+plataforma
              && pelota[1]-radioP <= -0.3 && pelota[1] > -0.3 ){ //arriba
          ySpeed = -ySpeed;
          pelota[1] += radioP;
      }    
  }
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

bool hayChoque(float x, float y, bool esEspecial){
    bool choca = false;
    bool esEsquina = false;

    if(pow ((x-pelota[0]),2) + pow(y-pelota[1],2) <= pow (radioP,2)){ // choca con esquina sup izq
        if(ySpeed < 0.0 && xSpeed > 0.0){
            ySpeed = -ySpeed;
            xSpeed = -xSpeed;
        }
        else{
          if(pelota[1] <= y) xSpeed = -xSpeed; 
          if(x <= pelota[0]) ySpeed = -ySpeed; 
        }
        choca = true;
        printf("esquina sup izq\n");
    } 
    else if(pow((x+lb-pelota[0]),2) + pow (y-pelota[1],2) <= pow(radioP,2)){// choca con la esquina sup der
        if(ySpeed < 0.0 && xSpeed < 0.0){
            ySpeed = -ySpeed;
            xSpeed = -xSpeed;
        }
        else{
          if(pelota[1] <= y) xSpeed = -xSpeed; //der 
          if(pelota[0] <= x+lb) ySpeed = -ySpeed; //arriba
        }
        choca = true;
        printf("esquina sup der\n");
    } 
    else if(pow ((x-pelota[0]),2) + pow(y-ab-pelota[1],2) <= pow (radioP,2)){// choca con la esquina inf izq
        if(ySpeed > 0.0 && xSpeed > 0.0){
            ySpeed = -ySpeed;
            xSpeed = -xSpeed;
        }
        else{
          if(pelota[1] >= y-ab) xSpeed = -xSpeed; 
          if(x <= pelota[0])  ySpeed = -ySpeed; //abajo
        }
        choca = true;
        printf("esquina inf der\n");
    } 
    else if(pow((x+lb-pelota[0]),2) + pow (y-ab-pelota[1],2) <= pow(radioP,2)){// choca con la esquina inf der
        if(ySpeed > 0.0 && xSpeed < 0.0){
            ySpeed = -ySpeed;
            xSpeed = -xSpeed;
        }
        else{
          if(pelota[1] >= y-ab) xSpeed = -xSpeed; //der
          if(x+lb >= pelota[0]) ySpeed = -ySpeed; //abajo
        }
        choca = true;
        printf("esquina inf izq\n");
    } 
    else if (pelota[0]-radioP <= x+lb && pelota[0]-radioP > x 
        && pelota[1] <= y && pelota[1] >= y-ab) {// choca del lado der del bloque
      xSpeed = -xSpeed;
      pelota[0] = x + lb + radioP;
      choca = true;
      printf("der\n");
    }
    else if (pelota[0]+radioP >= x && pelota[0]+radioP < x +lb 
        && pelota[1] <= y && pelota[1] >= y-ab) {// choca del lado izq del bloque
      xSpeed = -xSpeed;
      pelota[0] = x - radioP;
      choca = true;
      printf("izq\n");
    }
    else if (pelota[0] >= x && pelota[0] < x+lb && y >= pelota[1]-radioP 
        && pelota[1]-radioP >= y-ab) {//  choca de la parte de arriba del bloque
      ySpeed = -ySpeed;
      pelota[1] = y + radioP;
      choca = true;
      printf("arriba\n");
    } 
    else if (x <= pelota[0] && pelota[0] <= x+lb &&  pelota[1]+radioP <= y 
        && y-ab <= pelota[1]+radioP) {// choca de la parte de abajo del bloque
      ySpeed = -ySpeed;
      pelota[1] = y - ab - radioP;
      choca = true;
      printf("abajo\n");
    }

    return choca;
}

void dibujarBloques() {
  int esBonus, esEspecial;
  float cx = -8.4, cy = 15;

  glPushMatrix();
    glTranslatef(0.0,-8.2,0.0); //comparte eje con la pelota para revisar las colisiones mas facil

    for (int i = 0; i < 5; i++){
      for (int j = 0;j < 7;j++){

        esEspecial = buscarEspeciales(i*7+j);

        if (bloques[i][j] > -1){
          if(hayChoque(cx,cy,esEspecial)) bloques[i][j] +=1; //revisa si hay un choque
        }              

        switch (bloques[i][j]) {
          case -1: //bloque eliminado 
            esBonus = buscarBonus(i*7+j);
            if (!pausado && esBonus > -1 && posBonus[esBonus][1] > 0){
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
            if(!pausado && esEspecial > -1 && posEspeciales[esEspecial][2] < 5){
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
    //ojos
    glPointSize(30.0);
    glBegin(GL_POINTS);
      glVertex2f(-2.3,2.3);
      glVertex2f(2.3,2.3);
    glEnd();

    glPointSize(2.0);
    glPushMatrix();
      glScalef(0.8,0.8,0.8);
      if(gameOver){
          dibujarTexto(1);
          glColor3f(1,1,0);
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
          dibujarTexto(0);
          glColor3f(1,1,0);
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
    glutTimerFunc(10,movimientoPelota,1);
    glutPostRedisplay();
  }
}

/******************************* KEYBOARD *****************************/
void controlKey (unsigned char key, int xmouse, int ymouse){   
    switch (key){
        case ' ': // pausa juego
          if(!pausado && moviendose) pausado = true;
          else pausado = false;
        break;
        default: break;
    }
    glutPostRedisplay(); 
}

void initMovmiento(){
    glutTimerFunc(10,movimientoPelota,1);
    moviendose = true;
}

void handleSpecialKeypress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            isLeftKeyPressed = true;
            if (!isRightKeyPressed) {
                if(!pausado) plataforma -= vp; 
                if(!moviendose){
                  initMovmiento();
                  ySpeed = -ySpeed;
                  xSpeed = -xSpeed;
                }
            }
        break;
        case GLUT_KEY_RIGHT:
            isRightKeyPressed = true;
            if (!isLeftKeyPressed) {
                if(!pausado) plataforma += vp;
                if(!moviendose) initMovmiento();
            }
        break;
        default:
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
        default:
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

  if(!moviendose) dibujarTexto(2);
  if(pausado) dibujarTexto(3);

  if(destruidos < 35 && !gameOver){
    //------------- Dibujamos PLATAFORMA -------------          
        dibujarPlataforma();
    //------------- Dibujamos MARCO -------------
        dibujarMarcoVerde();
    //------------- Dibujamos PELOTA -------------
        dibujarPelota(0.3);
    //------------- Dibujamos BLOQUES -------------
        dibujarBloques();
  }else{
    dibujarCara();
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

    my_init();

    //GLenum err = glewInit();
    //if (GLEW_OK != err) {
    //  printf(stderr, "GLEW error");
    //  return 1;
    //}

    glutSpecialFunc(handleSpecialKeypress);
    glutSpecialUpFunc(handleSpecialKeyReleased);
    glutKeyboardFunc(controlKey);
    
    glutMainLoop();

    return 0;
}