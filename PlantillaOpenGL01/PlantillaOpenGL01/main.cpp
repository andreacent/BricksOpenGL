#include <iostream>
//#include <GL\glew.h>
//#include <GL\freeglut.h>
#include <math.h>
#include <stdlib.h>
#include <ctime>

#include <GL/freeglut.h>
#include <GL/gl.h>

#define PI 3.14159265358979f
#define lb 1.9        // largo del bloque
#define ab 0.5        // altura del bloque
#define velocidadP 0.00001 //velocidad de la pelota

/*
Las colisiones ya se ven naturales, pero la pelota siempre inicia en el mismo angulo. 
(me parece que esto se ve bien y podria dejarse asi en lugar de colocar el angulo de inicio aleatorio, 
porque aunque inicia en el mismo angulo, despues dependiendo del choque con la plataforma cambia 
de direccion y no hace siempre el mismo recorrido)
Los bonus y la explosión ya inician cerca del bloque correspondiente, sin embargo no se mueven.
Hay que buscar una forma de chequear la segunda colisión de los bloques especiales, ya que 
a veces con el primer choque rapidamente se pinta el bloque roto y desaparece, sin esperar 
la segunda colisión. Creo que esto mismo esta causando problemas tambien para pintar la 
explosion que a veces no aparece
*/

bool inicial = true, //true para inicializar los bonus y especiales una sola vez
     isLeftKeyPressed = false, isRightKeyPressed = false,
     velocidad = false, //bonus de velocidad activado o desactivado
     baseLarga = false,  //bonus de base activado o desactivado
     moviendose = false,
     gameOver = false;

float plataforma = 0.0,      //posicion de la plataforma
      //matriz de bloques
      // variables para cuando el bloque se rompe
      posInicial[8][2] = {{1,0},{0,0},{0,1},{0,0},{1,0},{1,0},{1,0},{0,0}}, 
      anguloPedazos[8] = {-330,-280,-90,-180,220,-60,320,-100},
      explota[2],
      bon[2] = {0.0,0.0}, // posicion del bonus
      tam = 2.0,
      // variables relacionadas a la pelota
      //pelota[2] = {0.0,0.0}, //posicion de la pelota
      anguloP = 40.0;          //angulo con el que se mueve la pelota

int bloques[5][7] = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},
                       {0,0,0,0,0,0,0},{0,0,0,0,0,0,0}},
    especiales[5] = {}, //arreglo para los bloques especiales 
    bonus[6][5]= {},    /*{posicion x, posicion y, tipo de bono} 
                          (0 tamaño de la plataforma, 1 velocidad de la pelota} */
    sumaGolpes,
    sumaBonus=0;

// Cosas de la pelota
GLfloat radioP = 0.3f;  // Radio de la pelota.
GLfloat pelota[2] = {0.0f,0.0f};      // Centro de la pelota.
GLfloat xSpeed = 0.08f;    // Velocidad en X y Direccion en Y.
GLfloat ySpeed = 0.05f;

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
bool buscarEspeciales(int x){
  bool esta = false;

  for (int i = 0; i < 5; i++){
    if (x == especiales[i]) {
      esta = true;
      break;
    }
  }
  return esta;
}

void generarEspeciales(){
    int r;
    for (int i = 0; i < 5; i++){
        r = rand()%35;
        while ( buscarEspeciales(r) ) r = rand()%35;
        especiales[i] = r;
    }
}

int buscarBonus(int x, int y){
  int esta = -1;

  for (int i = 0; i < 6; i++){
    if (x == bonus[i][0] && y == bonus[i][1]) {
      esta = i;
      break;
    }
  }
  return esta;
}

void generarBonus(){
    int i,j;

    for (int k = 0; k < 6; k++){  
      do{
          i = rand()%5;
          j = rand()%7;
      }while ( buscarBonus(i,j) >= 0 );
        bonus[k][0] = i;
        bonus[k][1] = j;
        bonus[k][2] = rand()%2;
        bonus[k][3] = 100;//j*2.5 - 8.4; //calcula posicion en x
        bonus[k][4] = 100;//15 - i*1.25; // calcula posicion en y
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

void dibujarExplosion(float x, float y){

    explota[0] = x;
    explota[1] = y;

    glPushMatrix();
      glTranslatef(explota[0]+0.75,explota[1]-0.25,0.0); 
      for (int i = 0; i < 8; i++){
        dibujarCirculo(posInicial[i][0], posInicial[i][1]);
        posInicial[i][0] = 0.1*cos(anguloPedazos[i])+ posInicial[i][0];
        posInicial[i][1] = 0.1*sin(anguloPedazos[i])+ posInicial[i][1];      
      }
      glutPostRedisplay();
    glPopMatrix();  
}

// -------- DIBUJOS PARA LA FORMA DE LOS BONUS  --------

void dibujarBonusVelocidad(float x, float y, int bono){ //
  GLfloat tp = tam+plataforma;

  glColor3f(1.0,0.5,0.0);
  glBegin(GL_LINE_LOOP);
    glVertex2f(x,y);
    glVertex2f(x+0.7,y);
    glVertex2f(x+0.6,y-0.5);
    glVertex2f(x+0.8,y-0.5);
    glVertex2f(x+0.5,y-1.0);
    glVertex2f(x+0.6,y-0.6);
    glVertex2f(x+0.3,y-0.6);
    glVertex2f(x+0.5,y-0.2);
  glEnd();

    // Colision con la plataforma
    if(  (y-1 <= -0.3 && tp >= x+0.5 && -tp <= x+0.5) // la punta choca con la plataforma
      || (y-0.5 <= -0.3 && y-0.5 >= -0.6 && (  (tp >= x+0.8 && -tp <= x+0.8) 
                                            || (tp >= x+0.6 && -tp <= x+0.6) )) //vertice a la altura (y-0.5) dentro de la plataforma
      || (y-0.6 <= -0.3 && y-0.6 >= -0.6 && (  (tp >= x+0.6 && -tp <= x+0.6) 
                                            || (tp >= x+0.3 && -tp <= x+0.3) )) //vertice a la altura (y-0.6) dentro de la plataforma
      || (y-0.2 <= -0.3 && y-0.2 >= -0.6 && ( tp >= x+0.5 || -tp <= x+0.5 ))) //vertice a la altura (y-0.2) dentro de la plataforma
    {
      if(velocidad) velocidad = false;
      else velocidad = true;
      bonus[bono][4] = -10.0;
      sumaBonus -= 1;
      dibujarCirculo(-2,4);
    }

}

void dibujarBonusTamBase(float x, float y, int bono){ // largo 0.8 en X, alto 0.2 en Y
    GLfloat tp = tam+plataforma;

    glColor3f(1.0,0.0,0.5);
    glBegin(GL_LINE_LOOP);
      glVertex2f(x+2,y-0.2);
      glVertex2f(x+2,y-0.4);
      glVertex2f(x+1,y-0.4);
      glVertex2f(x+1,y-0.2);
    glEnd();

    // Colision con la plataforma
    if(-tp <= x+1.0 && tp >= x+0.2 && y-0.4 <= -0.3){
      if(baseLarga) {
        baseLarga = false;
        tam -= 0.3;
      }
      else{
        tam += 0.3;
        baseLarga = true;
      }
      bonus[bono][4] = -10.0;
      sumaBonus -= 1;
      dibujarCirculo(-2,4);
    }

}

// -------------------- DIBUJOS BASE  ---------------------- 

void dibujarPlataforma() {

    glPushMatrix();
        glTranslatef(0.0,-8.2,0.0); 
        glColor3f(0.0,0.0,1.0);

        if(baseLarga){
          glBegin(GL_LINES);
              glVertex2f(-tam+plataforma+0.3, -0.3);
              glVertex2f(-tam+plataforma+0.3, -0.8);
              glVertex2f(tam+plataforma-0.3, -0.3);
              glVertex2f(tam+plataforma-0.3, -0.8);
          glEnd();
        } // base = 4 -> 15% = 0.6 -> base = 4.6

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

// --------FIN DIBUJO CUANDO EL BLOQUE SE ROMPE--------
void dibujarBloque(float cx, float cy, float color){

    glColor3f(1.0,color,0.0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cx,cy);
        glVertex2f(cx+lb,cy);
        glVertex2f(cx+lb,cy-0.5);
        glVertex2f(cx,cy-0.5);
    glEnd();

}

void dibujarBloqueRoto(float cx, float cy){ 

  glColor3f(0.8,1.0,0.8);
  glBegin(GL_LINE_LOOP);
    glVertex2f(cx,cy);
    glVertex2f(cx+lb/2,cy);
    glVertex2f(cx+lb/2-0.25,cy-0.15);
    glVertex2f(cx+lb/2+0.15,cy-0.4);
    glVertex2f(cx+lb/2,cy-0.5);
    glVertex2f(cx,cy-0.5);
  glEnd();
  glBegin(GL_LINE_LOOP);
    glVertex2f(cx+lb/2+0.1,cy);
    glVertex2f(cx+lb,cy);
    glVertex2f(cx+lb,cy-0.5);
    glVertex2f(cx+lb/2+0.15,cy-0.5);
    glVertex2f(cx+lb/2+0.25,cy-0.4);
    glVertex2f(cx+lb/2-0.1,cy-0.15);
  glEnd();

}

bool hayChoque(float x, float y){
    bool choca = false;
    float px = pelota[0], py = pelota[1];

    if (px-radioP <= x+lb && px-radioP > x && py <= y && py >= y-0.5) {// choca del lado der del bloque
      xSpeed = -xSpeed;
      px = x + lb + radioP;
      choca = true;
    }
    else if (px+radioP >= x && px+radioP < x +lb && py <= y && py >= y-0.5) {// choca del lado izq del bloque
      xSpeed = -xSpeed;
      px = x - radioP;
      choca = true;
    }
    else if (px >= x && px < x+lb && y >= py-radioP && py-radioP >= y-0.5) {//  choca de la parte de arriba del bloque
      ySpeed = -ySpeed;
      py = y + radioP;
      choca = true;
    } 
    else if (x <= px && px <= x+lb &&  py+radioP <= y && y-0.5 <= py+radioP) {// choca de la parte de abajo del bloque
      ySpeed = -ySpeed;
      py = y - 0.5 - radioP;
      choca = true;
    }
    else if(( pow ((x-px),2) + pow(y-py,2) == pow (radioP,2))         //  choca con la esquina sup izq bloque
           || (pow((x+lb-px),2) + pow (y-py,2) == pow(radioP,2))){  //  choca con la esquina sup der bloque
      ySpeed = -ySpeed;
      xSpeed = -xSpeed;
      choca = true;
    }

    else if (( pow ((x-px),2) + pow(y-0.5-py,2) == pow (radioP,2))          //  choca con la esquina inf izq bloque
            || (pow((x+lb-px),2) + pow (y-0.5-py,2) == pow(radioP,2))) {//  choca con la esquina inf der bloque
      xSpeed = -xSpeed;
      ySpeed = -ySpeed;
      choca = true;
    }
    pelota[0] = pelota[0] + xSpeed*velocidadP*cos(anguloP);
    pelota[1] = pelota[1] + ySpeed*velocidadP*sin(anguloP);

    return choca;
}

void movimientoB(int h){
  if (h > 0){
    //bonus
    if(sumaBonus > 0){
        for(int i=0; i<6; i++){
          if(bloques[bonus[i][0]][bonus[i][1]] == -1 && bonus[i][4] >= 0){ 
            bonus[i][4] -= 0.0000001;  
          }
          else if(bonus[i][4] < 0){
            sumaBonus -= 1;
          }
        }   
    }

    glutTimerFunc(5,movimientoB,1);
    glutPostRedisplay();
  }
}

void dibujarBloques() {
    int esBonus;
    glPushMatrix();
        glTranslatef(0.0,-8.2,0.0); //comparte eje con la pelota para revisar las colisiones mas facil
        float cx = -8.4, cy = 15;

        for (int i = 0; i < 5; i++){
            for (int j = 0;j < 7;j++){

              if (bloques[i][j] > -1){
                if(hayChoque(cx,cy)) bloques[i][j] +=1; //revisa si hay un choque
              }              

              // asignamos las posiciones a los bloques bonus
              esBonus = buscarBonus(i,j);
              if (esBonus > -1 && bonus[esBonus][3] == 100) {
                  bonus[esBonus][3] = cx;
                  bonus[esBonus][4] = cy;
                  //dibujarBonusTamBase(cx,cy);   
                  //dibujarBonusTamBase( bonus[esBonus][3],bonus[esBonus][4]); // deberian ser lo mismo pero no
                } 

              switch (bloques[i][j]) {
                case 0:
                  if(buscarEspeciales(i*7+j)) dibujarBloque(cx, cy, 1);//dibuja especiales
                  else dibujarBloque(cx, cy, 0);    
                break;
                case 1:
                  if(buscarEspeciales(i*7+j)) dibujarBloqueRoto(cx, cy);//dibuja especiales golpeados una vez
                  else{
                    bloques[i][j] = -1;
                    sumaGolpes +=1;
                    if (esBonus > -1) sumaBonus+=1; //incrementa para que se mueva el bonus
                  }
                break;
                case 2:
                  if(buscarEspeciales(i*7+j)){ 
                    dibujarExplosion(cx,cy);              
                    bloques[i][j] = -1;
                    sumaGolpes +=1;
                    if (esBonus > -1) sumaBonus+=1; //incrementa para que se mueva el bonus
                  }
                break;
                case -1:
                  if (esBonus > -1 && bonus[esBonus][4] > 0){
                    movimientoB(1);
                    switch (bonus[esBonus][2]) {
                      case 0:
                        dibujarBonusVelocidad(bonus[esBonus][3],bonus[esBonus][4],esBonus);    
                      break;
                      case 1:
                        dibujarBonusTamBase( bonus[esBonus][3],bonus[esBonus][4],esBonus);
                      break;
                    }
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

/***************** FIN FUNCIONES PARA DIBUJAR *****************/

/************************* MOVIMIENTO *************************/
void movimiento(int h){
  float v;
  if (h > 0){
    //pelota
    if(velocidad){
      v += velocidadP * 0.4;
      pelota[0] = v*cos(anguloP)+ pelota[0];
      pelota[1] = v*sin(anguloP)+ pelota[1];
    }else{      
      pelota[0] = velocidadP*cos(anguloP)+ pelota[0];
      pelota[1] = velocidadP*sin(anguloP)+ pelota[1];
    }

/*
    if(sumaBonus > 0){
        for(int i=0; i<6; i++){
          if(bloques[bonus[i][0]][bonus[i][1]] == -1 && bonus[i][4] >= 0){ 
            bonus[i][4] -= 0.0000001;  
          }
          else if(bonus[i][4] < 0){
            sumaBonus -= 1;
          }
        }   
    }
    */
    glutTimerFunc(10,movimiento,1);
    glutPostRedisplay();
  }
}


/************DIBUJA UNA CARA SI PIERDE O GANA************/
void dibujarCara(){
    float x,y;
    glColor3f(1,1,0);
    dibujarCirculo(0.0, 0.0, 6,0); // radio mas grandee

    glPointSize(30.0);
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


void handleSpecialKeypress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            isLeftKeyPressed = true;
            if (!isRightKeyPressed) {
                if (plataforma > -6.6 && !baseLarga) plataforma -= 0.2;
                else if (plataforma > -6.8 && !baseLarga) plataforma -= 0.1;
                else if (plataforma > -6.4 && baseLarga) plataforma -= 0.2;
                if (!moviendose){
                  anguloP = 95 + rand()% 25;
                  glutTimerFunc(10,movimiento,1);
                  moviendose = true;
                } 
            }
        break;
        case GLUT_KEY_RIGHT:
            isRightKeyPressed = true;
            if (!isLeftKeyPressed) {
                if (plataforma < 6.6 && !baseLarga) plataforma += 0.2;
                else if (plataforma < 6.8 && !baseLarga) plataforma += 0.1;
                else if (plataforma < 6.4 && baseLarga) plataforma += 0.2;
                if (!moviendose){
                  anguloP = 40 + rand()% 39;
                  glutTimerFunc(10,movimiento,1);
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
  // glutPostRedisplay(); Creo que en este caso no es necesario
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

  if(sumaGolpes < 40 && !gameOver){
    //------------- Dibujamos PLATAFORMA -------------          
        dibujarPlataforma();
    //------------- Dibujamos PELOTA -------------
        dibujarPelota(0.3);
    //------------- Dibujamos MARCO -------------
        dibujarMarcoVerde();
    //------------- Dibujamos BLOQUES -------------
        dibujarBloques();

  }

  else dibujarCara();

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